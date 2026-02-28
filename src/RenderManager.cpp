#include "gdxengine.h"
#include "RenderManager.h"
#include "Viewport.h"
#include "Light.h"
#include "Dx11RenderBackend.h"
#include "Dx11MaterialGpuData.h" 

RenderManager::RenderManager(ObjectManager& objectManager, LightManager& lightManager, GDXDevice& device)
    : m_objectManager(objectManager), m_lightManager(lightManager), m_device(device),
    m_currentCam(nullptr), m_directionLight(nullptr)
{
    // Schritt 1: DX11-Backend verwenden (copy + redirect, keine Behaviour-Aenderung)
    //m_backend = std::make_unique<Dx11RenderBackend>(m_device);

    Debug::Log("RenderManager.cpp: RenderManager erstellt - ShadowMapTarget + BackbufferTarget initialisiert");
}

RenderManager::~RenderManager()
{
    if (m_defaultSampler)
    {
        m_defaultSampler->Release();
        m_defaultSampler = nullptr;
    }
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
    m_activeRTT = rtt;
    m_rttCamera = rttCamera;
}

void RenderManager::UpdateShadowMatrixBuffer(const DirectX::XMMATRIX& viewMatrix,
    const DirectX::XMMATRIX& projMatrix)
{
    // Schritt 1: copy + redirect (keine Behaviour-Aenderung)
    if (m_backend) m_backend->UpdateShadowMatrixBuffer(m_device, viewMatrix, projMatrix);
}


void RenderManager::RenderShadowPass()
{
    if (!m_currentCam || !m_directionLight || !m_device.IsInitialized())
        return;

    Light* light = (m_directionLight->IsLight() ? m_directionLight->AsLight() : nullptr);
    if (!light) return;

    // Schritt 2: RenderTargets ins Backend verschoben (copy + redirect, keine Behaviour-Aenderung)
    if (m_backend) m_backend->BeginShadowPass();

    const DirectX::XMMATRIX lightViewMatrix = light->GetLightViewMatrix();
    const DirectX::XMMATRIX lightProjMatrix = light->GetLightProjectionMatrix();
    UpdateShadowMatrixBuffer(lightViewMatrix, lightProjMatrix);

    if (m_backend) m_backend->BindShadowMatrixConstantBufferVS(m_device);

    for (Mesh* mesh : m_objectManager.GetMeshes())
    {
        if (!mesh || mesh->m_surfaces.empty()) continue;
        if (!mesh->IsActive()) continue;
        if (!mesh->IsVisible()) continue;

        // Mesh-Level: wirft dieses Objekt Schatten?
        if (!mesh->GetCastShadows()) continue;

        MatrixSet ms = m_currentCam->matrixSet;
        ms.viewMatrix = lightViewMatrix;
        ms.projectionMatrix = lightProjMatrix;
        ms.worldMatrix = mesh->transform.GetLocalTransformationMatrix();
        mesh->Update(&m_device, &ms);

        for (Surface* s : mesh->m_surfaces)
        {
            if (!s) continue;

            Material* mat = s->pMaterial ? s->pMaterial : mesh->pMaterial;
            if (!mat || !mat->castShadows) continue;

            Shader* shader = mat->pRenderShader;
            if (!shader) continue;

            if (!shader->IsValid(ShaderBindMode::VS_ONLY))
            {
                Debug::LogError("RenderManager.cpp: RenderShadowPass – Shader ungueltig, Draw uebersprungen");
                continue;
            }
            shader->UpdateShader(&m_device, ShaderBindMode::VS_ONLY);
            s->gpu->Draw(&m_device, shader->flagsVertex);
        }
    }

    if (m_backend) m_backend->EndShadowPass();

}

void RenderManager::RenderNormalPass()
{
    // Kept for compatibility: historically this only did setup.
    // Now it executes the full (atomic) main pass.
    RenderMainPassAtomic();
}

void RenderManager::RenderMainPassAtomic()
{
    if (!m_currentCam || !m_device.IsInitialized() || !m_backend)
        return;

    const bool isRtt = (m_activeRTT != nullptr);

    // 1) Begin pass (bind RT/DS, viewport, RS etc.)
    if (isRtt)
    {
        m_backend->BeginRttPass(m_device, *m_activeRTT);
        Debug::LogOnce("RenderMainPassAtomic_RTT",
            "RenderManager.cpp: RenderMainPassAtomic - RTT-Pass aktiv");
    }
    else
    {
        const D3D11_VIEWPORT& dxVp = m_currentCam->viewport;
        Viewport vp;
        vp.x        = dxVp.TopLeftX;
        vp.y        = dxVp.TopLeftY;
        vp.width    = dxVp.Width;
        vp.height   = dxVp.Height;
        vp.minDepth = dxVp.MinDepth;
        vp.maxDepth = dxVp.MaxDepth;
        m_backend->BeginMainPass(m_device, m_backbufferTarget, vp);
    }

    // 2) Bind shadow resources / constants (unchanged slots + order)
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

    // 3) Per-frame lighting constants
    m_lightManager.Update(&m_device);

    // 4) Queue build + draw (kept)
    BuildRenderQueue();
    FlushRenderQueue();

    // 5) End pass (optional restore hooks)
    if (isRtt)
        m_backend->EndRttPass();
    else
        m_backend->EndMainPass();
}

void RenderManager::InvalidateFrame()
{
    m_opaque.Clear();
    //m_transparent.Clear();
}

void RenderManager::BuildRenderQueue()
{
    m_opaque.Clear();

    // Frame-Update-Flags aller Meshes zuruecksetzen
    for (Mesh* mesh : m_objectManager.GetMeshes())
        if (mesh) mesh->ResetFrameFlag();

    uint32_t cameraCullMask = LAYER_ALL;
    if (Camera* cam = (m_currentCam->IsCamera() ? m_currentCam->AsCamera() : nullptr))
        cameraCullMask = cam->cullMask;

    for (Mesh* mesh : m_objectManager.GetMeshes())
    {
        if (!mesh || mesh->m_surfaces.empty()) continue;
        if (!mesh->IsActive()) continue;
        if (!mesh->IsVisible()) continue;
        if (!(mesh->GetLayerMask() & cameraCullMask)) continue;

        const DirectX::XMMATRIX world = mesh->transform.GetLocalTransformationMatrix();

        // MatrixSet fuer Execute() vorbereiten -- world kommt aus dem Mesh-Transform
        mesh->matrixSet.worldMatrix = world;

        for (Surface* surface : mesh->m_surfaces)
        {
            if (!surface) continue;

            Material* material = surface->pMaterial ? surface->pMaterial : mesh->pMaterial;
            if (!material) continue;

            Shader* shader = material->pRenderShader;
            if (!shader) continue;

            m_opaque.Submit(shader, shader->flagsVertex, material, mesh, surface, world, m_backend.get());
        }
    }

    // Nach Shader/Material sortieren -- minimiert GPU-State-Wechsel
    m_opaque.Sort();

    static size_t s_lastCount = SIZE_MAX;
    if (m_opaque.Count() != s_lastCount)
    {
        s_lastCount = m_opaque.Count();
        Debug::Log("RenderManager.cpp: BuildRenderQueue - ", m_opaque.Count(), " RenderCommands");
    }
}

void RenderManager::FlushRenderQueue()
{
    Shader*   lastShader   = nullptr;
    Material* lastMaterial = nullptr;
    ID3D11DeviceContext* ctx = m_device.GetDeviceContext();

    // Sampler s0 einmalig binden (gilt fuer alle Materials / gSampler in PS)
    if (m_defaultSampler && ctx)
        ctx->PSSetSamplers(0, 1, &m_defaultSampler);

    for (RenderCommand& cmd : m_opaque.commands)
    {
        if (!cmd.shader || !cmd.material || !cmd.mesh || !cmd.surface) continue;

        // Shader nur binden wenn gewechselt
        if (cmd.shader != lastShader)
        {
            if (!cmd.shader->IsValid(ShaderBindMode::VS_PS))
            {
                Debug::LogError("RenderManager.cpp: FlushRenderQueue – Shader ungueltig, Draw uebersprungen");
                continue;
            }
            cmd.shader->UpdateShader(&m_device);
            lastShader = cmd.shader;
        }

        // Material nur binden wenn gewechselt
        if (cmd.material != lastMaterial)
        {
            // --- TexturePool-Pfad: feste Slots t0/t1/t2 aus Pool-Indizes ---
            // SM5.0 erlaubt kein dynamisches Texture-Array-Indexing im Shader.
            // Loesung: Pool verwaltet SRVs + Indizes, C++ bindet pro Material die
            // richtigen SRVs auf feste Slots (t0=Albedo, t1=Normal, t2=ORM).
            if (m_texturePool && ctx)
            {
                ID3D11ShaderResourceView* srvs[3] = {
                    m_texturePool->GetSRV(cmd.material->albedoIndex),
                    m_texturePool->GetSRV(cmd.material->normalIndex),
                    m_texturePool->GetSRV(cmd.material->ormIndex)
                };

                // Fallback auf White/FlatNormal/ORM falls Index ausserhalb Pool
                if (!srvs[0]) srvs[0] = m_texturePool->GetSRV(m_texturePool->WhiteIndex());
                if (!srvs[1]) srvs[1] = m_texturePool->GetSRV(m_texturePool->FlatNormalIndex());
                if (!srvs[2]) srvs[2] = m_texturePool->GetSRV(m_texturePool->OrmIndex());

                // Nur neu binden wenn sich die Slots geaendert haben (State-Cache)
                if (memcmp(srvs, m_boundSRVs, sizeof(srvs)) != 0)
                {
                    ctx->PSSetShaderResources(0, 3, srvs);
                    memcpy(m_boundSRVs, srvs, sizeof(srvs));
                }
            }
            else
            {
                // Fallback: alter Slot-Pfad (kein Pool aktiv)
                if (cmd.material->gpuData) cmd.material->gpuData->SetTexture(&m_device);
            }

            if (cmd.material->gpuData) cmd.material->gpuData->UpdateConstantBuffer(
                m_device.GetDeviceContext(),
                cmd.material->properties.baseColor,
                cmd.material->properties.specularColor,
                cmd.material->properties.emissiveColor,
                cmd.material->properties.uvTilingOffset,
                cmd.material->properties.metallic,
                cmd.material->properties.roughness,
                cmd.material->properties.normalScale,
                cmd.material->properties.occlusionStrength,
                cmd.material->properties.shininess,
                cmd.material->properties.transparency,
                cmd.material->properties.alphaCutoff,
                cmd.material->properties.receiveShadows,
                cmd.material->albedoIndex,
                cmd.material->normalIndex,
                cmd.material->ormIndex,
                cmd.material->decalIndex,
                cmd.material->properties.blendMode,
                cmd.material->properties.flags
            );
            lastMaterial = cmd.material;
        }

        // MatrixSet der Kamera in den Command schreiben und ausfuehren
        cmd.mesh->matrixSet = m_currentCam->matrixSet;
        cmd.mesh->matrixSet.worldMatrix = cmd.world;
        cmd.Execute(&m_device);
    }
}

void RenderManager::RenderScene()
{
    if (!m_currentCam)
        return;

    EnsureBackend();
    if (!m_backend)
        return;

    //if (!m_backend)
    //{
    //    if (!m_device.GetDevice() || !m_device.GetDeviceContext())
    //    {
    //        Debug::LogError("RenderManager::EnsureBackend - device not ready");
    //        return;
    //    }
    //
    //    m_backend = std::make_unique<Dx11RenderBackend>(m_device);
    //}

    // Wenn RTT aktiv, temporaer auf RTT-Kamera umschalten (falls gesetzt)
    LPENTITY savedCam = m_currentCam;
    if (m_activeRTT && m_rttCamera)
        m_currentCam = m_rttCamera;

    RenderShadowPass();
    RenderNormalPass();

    // Kamera wiederherstellen
    m_currentCam = savedCam;
}

void RenderManager::EnsureBackend()
{
    if (m_backend)
        return;

    // Device muss bereits existieren (nach InitializeDirectX)
    if (!m_device.GetDevice() || !m_device.GetDeviceContext())
    {
        Debug::LogError("RenderManager::EnsureBackend - device/context not ready.");
        return;
    }

    m_backend = std::make_unique<Dx11RenderBackend>(m_device);

    // Default-Sampler fuer gSampler (s0): linear, wrap, aniso 1
    if (!m_defaultSampler && m_device.GetDevice())
    {
        D3D11_SAMPLER_DESC sd{};
        sd.Filter   = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        sd.MaxAnisotropy = 1;
        sd.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        sd.MaxLOD = D3D11_FLOAT32_MAX;
        HRESULT hr = m_device.GetDevice()->CreateSamplerState(&sd, &m_defaultSampler);
        if (SUCCEEDED(hr))
            Debug::Log("RenderManager.cpp: Default-Sampler fuer s0 erstellt");
        else
            Debug::LogError("RenderManager.cpp: Default-Sampler Erstellung fehlgeschlagen");
    }
}