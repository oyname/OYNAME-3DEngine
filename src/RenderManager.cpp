#include "gdxengine.h"
#include "RenderManager.h"
#include "Viewport.h"
#include "Light.h"
#include "Dx11RenderBackend.h"
#include <unordered_map>

RenderManager::RenderManager(Scene& scene, AssetManager& assetManager, GDXDevice& device)
    : m_scene(scene), m_assetManager(assetManager), m_device(device),
      m_currentCam(nullptr), m_directionLight(nullptr)
{
    DBLOG("RenderManager.cpp: RenderManager created");
}

RenderManager::~RenderManager()
{
    // All DX11 state lives in Dx11RenderBackend and is released there.
}

void RenderManager::SetCamera(LPENTITY camera)
{
    m_currentCam = camera;
}

void RenderManager::SetDirectionalLight(LPENTITY dirLight)
{
    m_directionLight = dirLight;
}

void RenderManager::SetRTTTarget(RenderTextureTarget* rtt, LPENTITY rttCamera)
{
    m_activeRTT  = rtt;
    m_rttCamera  = rttCamera;
}

void RenderManager::UpdateShadowMatrixBuffer(const DirectX::XMMATRIX& viewMatrix,
                                              const DirectX::XMMATRIX& projMatrix)
{
    if (m_backend) m_backend->UpdateShadowMatrixBuffer(m_device, viewMatrix, projMatrix);
}

void RenderManager::LogFrameStatsIfChanged()
{
    // Entity stats live inside EntityGpuData (backend implementation detail).
    // Queried through the backend interface to keep RenderManager DX11-free.
    const IRenderBackend::EntityStats e = m_backend->GetEntityFrameStats();
    m_frameStats.entityUploads       = e.uploads;
    m_frameStats.entityConstantBinds = e.binds;
    m_frameStats.entityRingRotations = e.ringRotations;

    if (!m_hasLastLoggedFrameStats || m_frameStats != m_lastLoggedFrameStats)
    {
        DBLOG("RenderManager.cpp: Frame stats"
            " shadowDrawCalls=",      m_frameStats.shadowDrawCalls,
            " opaqueDrawCalls=",      m_frameStats.opaqueDrawCalls,
            " transparentDrawCalls=", m_frameStats.transparentDrawCalls,
            " shaderBinds=",          m_frameStats.shaderBinds,
            " materialBinds=",        m_frameStats.materialBinds,
            " entityUploads=",        m_frameStats.entityUploads,
            " entityCBBinds=",        m_frameStats.entityConstantBinds,
            " ringRotations=",        m_frameStats.entityRingRotations);

        m_lastLoggedFrameStats    = m_frameStats;
        m_hasLastLoggedFrameStats = true;
    }
}

void RenderManager::RenderShadowPass()
{
    if (!m_currentCam || !m_directionLight || !m_device.IsInitialized() || !m_backend)
        return;

    Light* light = (m_directionLight->IsLight() ? m_directionLight->AsLight() : nullptr);
    if (!light) return;

    m_backend->BeginShadowPass();

    const DirectX::XMMATRIX lightViewMatrix = light->GetLightViewMatrix();
    const DirectX::XMMATRIX lightProjMatrix = light->GetLightProjectionMatrix();
    UpdateShadowMatrixBuffer(lightViewMatrix, lightProjMatrix);
    m_backend->BindShadowMatrixConstantBufferVS(m_device);

    BuildShadowQueue();
    FlushShadowQueue(lightViewMatrix, lightProjMatrix);

    m_backend->EndShadowPass();
}

void RenderManager::RenderNormalPass()
{
    // Kept for compatibility: delegates to the atomic main pass.
    RenderMainPassAtomic();
}

void RenderManager::RenderMainPassAtomic()
{
    if (!m_currentCam || !m_device.IsInitialized() || !m_backend)
        return;

    const bool isRtt = (m_activeRTT != nullptr);

    // 1) Bind render target, viewport, rasterizer state
    if (isRtt)
    {
        m_backend->BeginRttPass(m_device, *m_activeRTT);
        DBLOG_ONCE("RenderMainPassAtomic_RTT",
            "RenderManager.cpp: RenderMainPassAtomic - RTT pass active");
    }
    else
    {
        m_backend->BeginMainPass(m_device, m_backbufferTarget, m_currentCam->viewport);
    }

    // 2) Shadow resources + matrix constants
    m_backend->BindShadowResourcesPS(m_device, m_shadowTarget);

    if (m_directionLight)
    {
        Light* light = (m_directionLight->IsLight() ? m_directionLight->AsLight() : nullptr);
        if (light)
        {
            UpdateShadowMatrixBuffer(light->GetLightViewMatrix(), light->GetLightProjectionMatrix());
            m_backend->BindShadowMatrixConstantBufferVS(m_device);
        }
    }

    // 3) Light array constant buffer (b1)
    {
        DirectX::XMFLOAT4 globalAmbient(0.2f, 0.2f, 0.2f, 1.0f);
        if (GDXEngine::GetInstance())
            globalAmbient = GDXEngine::GetInstance()->GetGlobalAmbient();

        m_backend->UploadLightConstants(m_scene.GetLights(), globalAmbient);
    }

    // 4) Queue build + draw
    BuildRenderQueue();
    FlushRenderQueue();
    FlushTransparentQueue();

    // 5) End pass
    if (isRtt)
        m_backend->EndRttPass();
    else
        m_backend->EndMainPass();
}

void RenderManager::InvalidateFrame()
{
    m_opaque.Clear();
    m_shadow.Clear();
    m_transFrame.clear();
    m_frameStats = {};
    m_flushOnce  = false;

    // Entity frame stats are backend-owned; reset through the interface.
    m_backend->ResetEntityFrameStats();

    for (Mesh* mesh : m_scene.GetMeshes())
        if (mesh) mesh->ResetFrameFlag();
}

void RenderManager::BuildShadowQueue()
{
    m_shadow.Clear();

    uint32_t cameraCullMask = LAYER_ALL;
    if (Camera* cam = (m_currentCam->IsCamera() ? m_currentCam->AsCamera() : nullptr))
        cameraCullMask = cam->cullMask;

    for (Mesh* mesh : m_scene.GetMeshes())
    {
        if (!mesh || !mesh->HasMeshAsset()) continue;
        if (mesh->GetSurfaces().empty())    continue;
        if (!mesh->IsActive())              continue;
        if (!mesh->IsVisible())             continue;
        if (!mesh->GetCastShadows())        continue;
        if (!(mesh->GetLayerMask() & cameraCullMask)) continue;

        const DirectX::XMMATRIX world = mesh->GetWorldMatrix();

        const auto& shadowSlots = mesh->GetSurfaces();
        for (unsigned int si = 0; si < static_cast<unsigned int>(shadowSlots.size()); ++si)
        {
            Surface* surface = shadowSlots[si];
            if (!surface) continue;

            Material* material = mesh->GetResolvedMaterial(si,
                m_assetManager.GetStandardMaterial());
            if (!material || !material->GetCastShadows()) continue;

            Shader* shader = material->pRenderShader;
            if (!shader) continue;

            if (!shader->IsValid(ShaderBindMode::VS_ONLY))
            {
                DBERROR("RenderManager.cpp: BuildShadowQueue - invalid shader, draw skipped");
                continue;
            }

            m_shadow.Submit(shader, shader->flagsVertex, material, mesh, surface, world, m_backend.get());
        }
    }

    m_shadow.Sort();

    static size_t s_lastShadowCount = static_cast<size_t>(-1);
    if (m_shadow.Count() != s_lastShadowCount)
    {
        s_lastShadowCount = m_shadow.Count();
        DBLOG("RenderManager.cpp: BuildShadowQueue count=", m_shadow.Count());

        for (size_t i = 0; i < m_shadow.commands.size(); ++i)
        {
            const RenderCommand& cmd = m_shadow.commands[i];
            DBLOG("RenderManager.cpp:   shadow[", (int)i, "]"
                " shader=", (void*)cmd.shader,
                " mat=",    (void*)cmd.material,
                " mesh=",   (void*)cmd.mesh,
                " surface=",(void*)cmd.surface);
        }
    }
}

void RenderManager::BuildRenderQueue()
{
    m_opaque.Clear();
    m_transFrame.clear();

    // Skinned meshes: reset frame flag because their shadow pass writes light
    // matrices to b0. Non-skinned meshes keep the flag (camera matrices are
    // already correct from the shadow VS pass).
    for (Mesh* mesh : m_scene.GetMeshes())
        if (mesh && mesh->hasSkinning) mesh->ResetFrameFlag();

    uint32_t cameraCullMask = LAYER_ALL;
    if (Camera* cam = (m_currentCam->IsCamera() ? m_currentCam->AsCamera() : nullptr))
        cameraCullMask = cam->cullMask;

    DirectX::XMVECTOR camPos = m_currentCam->GetWorldMatrix().r[3];

    for (Mesh* mesh : m_scene.GetMeshes())
    {
        if (!mesh || !mesh->HasMeshAsset()) continue;
        if (mesh->GetSurfaces().empty())    continue;
        if (!mesh->IsActive())              continue;
        if (!mesh->IsVisible())             continue;
        if (!(mesh->GetLayerMask() & cameraCullMask)) continue;

        const DirectX::XMMATRIX world = mesh->GetWorldMatrix();
        mesh->matrixSet.worldMatrix   = world;

        const auto& queueSlots = mesh->GetSurfaces();
        for (unsigned int qi = 0; qi < static_cast<unsigned int>(queueSlots.size()); ++qi)
        {
            Surface* surface = queueSlots[qi];
            if (!surface) continue;

            Material* material = mesh->GetResolvedMaterial(qi,
                m_assetManager.GetStandardMaterial());
            if (!material) continue;

            Shader* shader = material->pRenderShader;

            DBLOG_ONCE("STD_MAT_CHECK",
                "STD=",     (void*)m_assetManager.GetStandardMaterial(),
                " shader=", (void*)(m_assetManager.GetStandardMaterial() ? m_assetManager.GetStandardMaterial()->pRenderShader : nullptr),
                " gpu=",    (void*)(m_assetManager.GetStandardMaterial() ? m_assetManager.GetStandardMaterial()->gpuData : nullptr));

            if (!shader) continue;

            if (material->IsTransparent())
            {
                DirectX::XMVECTOR meshPos = world.r[3];
                DirectX::XMVECTOR diff    = DirectX::XMVectorSubtract(meshPos, camPos);
                float depth = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(diff));

                RenderCommand cmd;
                cmd.mesh        = mesh;
                cmd.surface     = surface;
                cmd.world       = world;
                cmd.shader      = shader;
                cmd.material    = material;
                cmd.flagsVertex = shader->flagsVertex;
                cmd.backend     = m_backend.get();
                m_transFrame.emplace_back(depth, cmd);
            }
            else
            {
                m_opaque.Submit(shader, shader->flagsVertex, material, mesh, surface, world, m_backend.get());
            }
        }
    }

    m_opaque.Sort();

    std::sort(m_transFrame.begin(), m_transFrame.end(),
        [](const auto& a, const auto& b) { return a.first > b.first; });

    static std::unordered_map<void*, size_t> s_lastOpaque;
    static std::unordered_map<void*, size_t> s_lastTrans;

    void* camKey = static_cast<void*>(m_currentCam);

    if (m_opaque.Count() != s_lastOpaque[camKey] ||
        m_transFrame.size() != s_lastTrans[camKey])
    {
        s_lastOpaque[camKey] = m_opaque.Count();
        s_lastTrans[camKey]  = m_transFrame.size();

        DBLOG("RenderManager.cpp: BuildRenderQueue [cam=", camKey, "]"
            " opaque=",      m_opaque.Count(),
            " transparent=", m_transFrame.size());

        for (size_t i = 0; i < m_opaque.commands.size(); ++i)
        {
            const RenderCommand& cmd = m_opaque.commands[i];
            DBLOG("RenderManager.cpp:   opaque[", (int)i, "]"
                " shader=", (void*)cmd.shader,
                " mat=",    (void*)cmd.material,
                " mesh=",   (void*)cmd.mesh,
                " surface=",(void*)cmd.surface);
        }

        for (size_t i = 0; i < m_transFrame.size(); ++i)
        {
            const RenderCommand& cmd = m_transFrame[i].second;
            DBLOG("RenderManager.cpp:   trans[", (int)i, "]"
                " depth=", m_transFrame[i].first,
                " mat=",   (void*)cmd.material,
                " mesh=",  (void*)cmd.mesh);
        }
    }
}

void RenderManager::FlushShadowQueue(const DirectX::XMMATRIX& lightViewMatrix,
                                      const DirectX::XMMATRIX& lightProjMatrix)
{
    unsigned int shaderBinds = 0;
    unsigned int drawCalls   = 0;

    Shader* lastShader = nullptr;

    for (auto& cmd : m_shadow.commands)
    {
        if (!cmd.shader || !cmd.mesh || !cmd.surface) continue;

        // Non-skinned: dedicated shadow VS reads world from b0, light VP from b3.
        // Skinned: material VS in VS_ONLY mode writes light matrices to b0.
        const bool useShadowVS  = (m_shadowShader != nullptr && !cmd.mesh->hasSkinning);
        Shader*    activeShader = useShadowVS ? m_shadowShader : cmd.shader;

        if (activeShader != lastShader)
        {
            if (!activeShader->IsValid(ShaderBindMode::VS_ONLY))
            {
                DBERROR("RenderManager.cpp: FlushShadowQueue - invalid shader, draw skipped");
                continue;
            }

            activeShader->UpdateShader(&m_device, ShaderBindMode::VS_ONLY);
            lastShader = activeShader;
            ++shaderBinds;
        }

        if (useShadowVS)
        {
            cmd.mesh->matrixSet = m_currentCam->matrixSet;
            cmd.mesh->matrixSet.worldMatrix = cmd.world;
        }
        else
        {
            cmd.mesh->matrixSet = m_currentCam->matrixSet;
            cmd.mesh->matrixSet.viewMatrix       = lightViewMatrix;
            cmd.mesh->matrixSet.projectionMatrix = lightProjMatrix;
            cmd.mesh->matrixSet.worldMatrix      = cmd.world;
        }

        cmd.Execute(&m_device);
        ++drawCalls;
    }

    m_frameStats.shaderBinds     += shaderBinds;
    m_frameStats.shadowDrawCalls += drawCalls;
}

void RenderManager::FlushRenderQueue()
{
    unsigned int shaderBinds   = 0;
    unsigned int materialBinds = 0;
    unsigned int drawCalls     = 0;

    Shader*   lastShader   = nullptr;
    Material* lastMaterial = nullptr;

    m_backend->ResetMaterialCache();
    m_backend->BindFrameSampler();

    for (auto& cmd : m_opaque.commands)
    {
        if (!cmd.shader || !cmd.material || !cmd.mesh || !cmd.surface) continue;

        if (cmd.shader != lastShader)
        {
            if (!cmd.shader->IsValid(ShaderBindMode::VS_PS))
            {
                DBERROR("RenderManager.cpp: FlushRenderQueue - invalid shader, draw skipped");
                continue;
            }

            cmd.shader->UpdateShader(&m_device);
            lastShader = cmd.shader;
            ++shaderBinds;
        }

        if (cmd.material != lastMaterial)
        {
            m_backend->BindMaterial(cmd.material, m_texturePool);
            lastMaterial = cmd.material;
            ++materialBinds;
        }

        ++drawCalls;

        cmd.mesh->matrixSet = m_currentCam->matrixSet;
        cmd.mesh->matrixSet.worldMatrix = cmd.world;
        cmd.Execute(&m_device);
    }

    m_frameStats.shaderBinds     += shaderBinds;
    m_frameStats.materialBinds   += materialBinds;
    m_frameStats.opaqueDrawCalls += drawCalls;
}

void RenderManager::FlushTransparentQueue()
{
    if (m_transFrame.empty()) return;

    unsigned int shaderBinds   = 0;
    unsigned int materialBinds = 0;
    unsigned int drawCalls     = 0;

    m_backend->ResetMaterialCache();
    m_backend->BindFrameSampler();
    m_backend->SetAlphaBlend(true);

    Shader*   lastShader   = nullptr;
    Material* lastMaterial = nullptr;

    for (auto& [depth, cmd] : m_transFrame)
    {
        (void)depth;

        if (!cmd.shader || !cmd.material || !cmd.mesh || !cmd.surface) continue;

        if (cmd.shader != lastShader)
        {
            if (!cmd.shader->IsValid(ShaderBindMode::VS_PS)) continue;
            cmd.shader->UpdateShader(&m_device);
            lastShader = cmd.shader;
            ++shaderBinds;
        }

        if (cmd.material != lastMaterial)
        {
            m_backend->BindMaterial(cmd.material, m_texturePool);
            lastMaterial = cmd.material;
            ++materialBinds;
        }

        ++drawCalls;

        cmd.mesh->matrixSet = m_currentCam->matrixSet;
        cmd.mesh->matrixSet.worldMatrix = cmd.world;
        cmd.Execute(&m_device);
    }

    m_frameStats.shaderBinds          += shaderBinds;
    m_frameStats.materialBinds        += materialBinds;
    m_frameStats.transparentDrawCalls += drawCalls;

    m_backend->SetAlphaBlend(false);
}

void RenderManager::RenderScene()
{
    if (!m_currentCam) return;

    // EnsureBackend first: InvalidateFrame delegates stats reset to the backend,
    // so the backend must exist before InvalidateFrame is called.
    EnsureBackend();
    if (!m_backend) return;

    InvalidateFrame();

    LPENTITY savedCam = m_currentCam;
    if (m_activeRTT && m_rttCamera)
        m_currentCam = m_rttCamera;

    RenderShadowPass();
    RenderNormalPass();

    m_currentCam = savedCam;

    LogFrameStatsIfChanged();
}

void RenderManager::EnsureBackend()
{
    if (m_backend) return;

    if (!m_device.IsInitialized())
    {
        DBERROR("RenderManager.cpp: EnsureBackend - device not ready");
        return;
    }

    m_backend = std::make_unique<Dx11RenderBackend>(m_device);
}
