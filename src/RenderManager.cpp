#include "gdxengine.h"
#include "RenderManager.h"
#include "Light.h"

RenderManager::RenderManager(ObjectManager& objectManager, LightManager& lightManager, GDXDevice& device)
    : m_objectManager(objectManager), m_lightManager(lightManager), m_device(device),
    m_currentCam(nullptr), m_directionLight(nullptr)
{
    m_shadowTarget.SetDevice(&m_device);
    m_backbufferTarget.SetDevice(&m_device);

    Debug::Log("RenderManager.cpp: RenderManager erstellt - ShadowMapTarget + BackbufferTarget initialisiert");
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
    if (!m_device.IsInitialized()) return;

    ID3D11Buffer* shadowMatrixBuffer = m_device.GetShadowMatrixBuffer();
    if (!shadowMatrixBuffer) return;

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = m_device.GetDeviceContext()->Map(
        shadowMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(hr)) return;

    struct ShadowMatrixBuffer
    {
        DirectX::XMMATRIX lightViewMatrix;
        DirectX::XMMATRIX lightProjectionMatrix;
    };

    constexpr bool HLSL_USES_ROW_MAJOR = true;

    ShadowMatrixBuffer* bufferData = reinterpret_cast<ShadowMatrixBuffer*>(mappedResource.pData);
    if (HLSL_USES_ROW_MAJOR)
    {
        bufferData->lightViewMatrix = viewMatrix;
        bufferData->lightProjectionMatrix = projMatrix;
    }
    else
    {
        bufferData->lightViewMatrix = DirectX::XMMatrixTranspose(viewMatrix);
        bufferData->lightProjectionMatrix = DirectX::XMMatrixTranspose(projMatrix);
    }

    m_device.GetDeviceContext()->Unmap(shadowMatrixBuffer, 0);
}

void RenderManager::RenderShadowPass()
{
    if (!m_currentCam || !m_directionLight || !m_device.IsInitialized())
        return;

    Light* light = (m_directionLight->IsLight() ? m_directionLight->AsLight() : nullptr);
    if (!light) return;

    m_shadowTarget.Bind();
    m_shadowTarget.Clear();

    const DirectX::XMMATRIX lightViewMatrix = light->GetLightViewMatrix();
    const DirectX::XMMATRIX lightProjMatrix = light->GetLightProjectionMatrix();
    UpdateShadowMatrixBuffer(lightViewMatrix, lightProjMatrix);

    if (ID3D11Buffer* shadowMatrixBuffer = m_device.GetShadowMatrixBuffer())
        m_device.GetDeviceContext()->VSSetConstantBuffers(3, 1, &shadowMatrixBuffer);

    for (Mesh* mesh : m_objectManager.GetMeshes())
    {
        if (!mesh || mesh->surfaces.empty()) continue;
        if (!mesh->IsActive()) continue;
        if (!mesh->IsVisible()) continue;

        // Mesh-Level: wirft dieses Objekt Schatten?
        if (!mesh->GetCastShadows()) continue;

        MatrixSet ms = m_currentCam->matrixSet;
        ms.viewMatrix = lightViewMatrix;
        ms.projectionMatrix = lightProjMatrix;
        ms.worldMatrix = mesh->transform.GetLocalTransformationMatrix();
        mesh->Update(&m_device, &ms);

        for (Surface* s : mesh->surfaces)
        {
            if (!s) continue;

            Material* mat = s->pMaterial ? s->pMaterial : mesh->pMaterial;
            if (!mat || !mat->castShadows) continue;

            Shader* shader = mat->pRenderShader;
            if (!shader) continue;

            shader->UpdateShader(&m_device, ShaderBindMode::VS_ONLY);
            s->Draw(&m_device, shader->flagsVertex);
        }
    }
}

void RenderManager::RenderNormalPass()
{
    if (!m_currentCam || !m_device.IsInitialized())
        return;

    ID3D11DeviceContext* ctx = m_device.GetDeviceContext();
    if (!ctx) return;

    if (m_activeRTT)
    {
        // RTT-Pass: in die Render-Textur rendern
        m_activeRTT->Bind();
        m_activeRTT->Clear();

        Debug::LogOnce("RenderNormalPass_RTT",
            "RenderManager.cpp: RenderNormalPass - RTT-Pass aktiv");
    }
    else
    {
        // Standard-Pass: in den Backbuffer rendern
        m_backbufferTarget.SetViewport(m_currentCam->viewport);
        m_backbufferTarget.Bind();
    }

    constexpr UINT SHADOW_TEX_SLOT = 7;
    ID3D11ShaderResourceView* shadowSRV = m_shadowTarget.GetSRV();
    ID3D11SamplerState* shadowSmp = m_device.GetComparisonSampler();
    ctx->PSSetShaderResources(SHADOW_TEX_SLOT, 1, &shadowSRV);
    ctx->PSSetSamplers(SHADOW_TEX_SLOT, 1, &shadowSmp);

    if (m_directionLight)
    {
        Light* light = (m_directionLight->IsLight() ? m_directionLight->AsLight() : nullptr);
        if (light)
        {
            UpdateShadowMatrixBuffer(light->GetLightViewMatrix(), light->GetLightProjectionMatrix());

            if (ID3D11Buffer* shadowMatrixBuffer = m_device.GetShadowMatrixBuffer())
                ctx->VSSetConstantBuffers(3, 1, &shadowMatrixBuffer);
        }
    }
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
        if (!mesh || mesh->surfaces.empty()) continue;
        if (!mesh->IsActive()) continue;
        if (!mesh->IsVisible()) continue;
        if (!(mesh->GetLayerMask() & cameraCullMask)) continue;

        const DirectX::XMMATRIX world = mesh->transform.GetLocalTransformationMatrix();

        for (Surface* surface : mesh->surfaces)
        {
            if (!surface) continue;

            Material* material = surface->pMaterial ? surface->pMaterial : mesh->pMaterial;
            if (!material) continue;

            Shader* shader = material->pRenderShader;
            if (!shader) continue;

            m_opaque.Submit(shader, shader->flagsVertex, material, mesh, surface, world);
        }
    }

    // Queue-Inhalt einmalig loggen (nur wenn sich Shader-Bucket-Anzahl aendert)
    static size_t s_lastShaderCount = SIZE_MAX;
    size_t totalDraws = 0;
    for (auto& sb : m_opaque.shaders)
        for (auto& mb : sb.materials)
            totalDraws += mb.draws.size();

    if (m_opaque.shaders.size() != s_lastShaderCount)
    {
        s_lastShaderCount = m_opaque.shaders.size();

        Debug::Log("=== RenderQueue ===");
        Debug::Log("  Shader-Buckets: ", m_opaque.shaders.size());

        for (size_t si = 0; si < m_opaque.shaders.size(); ++si)
        {
            ShaderBatch& sb = m_opaque.shaders[si];
            Debug::Log("  [Shader ", si, "] ptr=", Ptr(sb.shader).c_str(),
                "  Material-Buckets: ", sb.materials.size());

            for (size_t mi = 0; mi < sb.materials.size(); ++mi)
            {
                MaterialBatch& mb = sb.materials[mi];
                Debug::Log("    [Material ", mi, "] ptr=", Ptr(mb.material).c_str(),
                    "  DrawEntries: ", mb.draws.size());

                for (size_t di = 0; di < mb.draws.size(); ++di)
                {
                    Debug::Log("      [Draw ", di, "] mesh=", Ptr(mb.draws[di].mesh).c_str(),
                        "  surface=", Ptr(mb.draws[di].surface).c_str());
                }
            }
        }

        Debug::Log("  Gesamt Draw Calls: ", totalDraws);
        Debug::Log("===================");
    }
}

void RenderManager::FlushRenderQueue()
{
    // Debug-Log: einmalig ausgeben wenn sich Queue-Struktur aendert.
    // Zeigt pro Shader/Material/Draw ob Mesh voll geupdated oder nur CB-gebunden wird.
    static size_t s_lastFlushShaderCount = SIZE_MAX;
    bool doLog = (m_opaque.shaders.size() != s_lastFlushShaderCount);
    if (doLog)
    {
        s_lastFlushShaderCount = m_opaque.shaders.size();
        Debug::Log("=== FlushRenderQueue ===");
    }

    for (size_t si = 0; si < m_opaque.shaders.size(); ++si)
    {
        ShaderBatch& sb = m_opaque.shaders[si];
        if (!sb.shader) continue;

        sb.shader->UpdateShader(&m_device);

        if (doLog)
            Debug::Log("  [Shader ", si, "] ptr=", Ptr(sb.shader).c_str(),
                "  flagsVertex=", sb.flagsVertex);

        for (size_t mi = 0; mi < sb.materials.size(); ++mi)
        {
            MaterialBatch& mb = sb.materials[mi];
            if (!mb.material) continue;

            mb.material->SetTexture(&m_device);
            mb.material->UpdateConstantBuffer(m_device.GetDeviceContext());

            if (doLog)
                Debug::Log("    [Material ", mi, "] ptr=", Ptr(mb.material).c_str(),
                    "  Draws: ", mb.draws.size());

            for (size_t di = 0; di < mb.draws.size(); ++di)
            {
                DrawEntry& entry = mb.draws[di];
                if (!entry.mesh || !entry.surface) continue;

                if (!entry.mesh->IsUpdatedThisFrame())
                {
                    // Erster Auftritt dieses Mesh: voller Update (Matrix + CB-Upload + Bind)
                    entry.mesh->Update(&m_device, &m_currentCam->matrixSet);
                    entry.mesh->MarkUpdated();

                    if (doLog)
                        Debug::Log("      [Draw ", di, "] mesh=", Ptr(entry.mesh).c_str(),
                            "  surface=", Ptr(entry.surface).c_str(),
                            "  -> FULL UPDATE (matrix + CB upload + bind)");
                }
                else
                {
                    // Mesh schon geupdated: nur CB binden, kein Map/Unmap
                    ID3D11Buffer* cb = entry.mesh->constantBuffer;
                    if (cb)
                    {
                        m_device.GetDeviceContext()->VSSetConstantBuffers(0, 1, &cb);
                        m_device.GetDeviceContext()->PSSetConstantBuffers(0, 1, &cb);
                    }

                    if (doLog)
                        Debug::Log("      [Draw ", di, "] mesh=", Ptr(entry.mesh).c_str(),
                            "  surface=", Ptr(entry.surface).c_str(),
                            "  -> CB BIND ONLY (matrix already uploaded)");
                }

                entry.surface->Draw(&m_device, sb.flagsVertex);
            }
        }
    }

    if (doLog)
        Debug::Log("========================");
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