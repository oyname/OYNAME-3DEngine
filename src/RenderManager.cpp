#include "gdxengine.h"
#include "RenderManager.h"
#include "Viewport.h"
#include "Light.h"

#include "Dx11RenderBackend.h"

RenderManager::RenderManager(ObjectManager& objectManager, LightManager& lightManager, GDXDevice& device)
    : m_objectManager(objectManager), m_lightManager(lightManager), m_device(device),
    m_currentCam(nullptr), m_directionLight(nullptr)
{
    // Schritt 1: DX11-Backend verwenden (copy + redirect, keine Behaviour-Aenderung)
    m_backend = std::make_unique<Dx11RenderBackend>();

    Debug::Log("RenderManager.cpp: RenderManager erstellt - ShadowMapTarget + BackbufferTarget initialisiert");
}

RenderManager::~RenderManager() = default;

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
    if (m_backend) m_backend->BeginShadowPass(m_device, m_shadowTarget);

    const DirectX::XMMATRIX lightViewMatrix = light->GetLightViewMatrix();
    const DirectX::XMMATRIX lightProjMatrix = light->GetLightProjectionMatrix();
    UpdateShadowMatrixBuffer(lightViewMatrix, lightProjMatrix);

    m_backend->BindShadowMatrixConstantBufferVS(m_device);

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

            shader->UpdateShader(&m_device, ShaderBindMode::VS_ONLY);
            s->gpu->Draw(&m_device, shader->flagsVertex);
        }
    }
}

void RenderManager::RenderNormalPass()
{
    if (!m_currentCam || !m_device.IsInitialized())
        return;

    if (m_activeRTT)
    {
        // RTT-Pass: in die Render-Textur rendern
        // Schritt 2: RenderTargets ins Backend verschoben (copy + redirect, keine Behaviour-Aenderung)
        if (m_backend) m_backend->BeginRttPass(m_device, *m_activeRTT);

        Debug::LogOnce("RenderNormalPass_RTT",
            "RenderManager.cpp: RenderNormalPass - RTT-Pass aktiv");
    }
    else
    {
        // Standard-Pass: in den Backbuffer rendern
        // Schritt 2: RenderTargets ins Backend verschoben (copy + redirect, keine Behaviour-Aenderung)
        if (m_backend)
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
    }

    // Schritt 1: Shadow SRV + Comparison-Sampler binden (copy + redirect, Slots unveraendert)
    if (m_backend) m_backend->BindShadowResourcesPS(m_device, m_shadowTarget);

    if (m_directionLight)
    {
        Light* light = (m_directionLight->IsLight() ? m_directionLight->AsLight() : nullptr);
        if (light)
        {
            // Schritt 1: Shadow-Matrix-Buffer updaten + binden (copy + redirect, Slot b3 unveraendert)
            UpdateShadowMatrixBuffer(light->GetLightViewMatrix(), light->GetLightProjectionMatrix());
            if (m_backend) m_backend->BindShadowMatrixConstantBufferVS(m_device);
        }
    }
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

    for (RenderCommand& cmd : m_opaque.commands)
    {
        if (!cmd.shader || !cmd.material || !cmd.mesh || !cmd.surface) continue;

        // Shader nur binden wenn gewechselt
        if (cmd.shader != lastShader)
        {
            cmd.shader->UpdateShader(&m_device);
            lastShader = cmd.shader;
        }

        // Material nur binden wenn gewechselt
        if (cmd.material != lastMaterial)
        {
            cmd.material->SetTexture(&m_device);
            cmd.material->UpdateConstantBuffer(m_device.GetDeviceContext());
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

    // Wenn RTT aktiv, temporaer auf RTT-Kamera umschalten (falls gesetzt)
    LPENTITY savedCam = m_currentCam;
    if (m_activeRTT && m_rttCamera)
        m_currentCam = m_rttCamera;

    RenderShadowPass();
    RenderNormalPass();

    m_lightManager.Update(&m_device);

    Debug::LogOnce("RenderScene_Camera",
        "RenderManager.cpp: RenderScene - Camera: ", Ptr(m_currentCam).c_str());

    BuildRenderQueue();
    FlushRenderQueue();

    // Kamera wiederherstellen
    m_currentCam = savedCam;
}