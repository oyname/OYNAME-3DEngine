#include "gdxengine.h"
#include "RenderManager.h"
#include "Light.h"

RenderManager::RenderManager(ObjectManager& objectManager, LightManager& lightManager, GDXDevice& device)
    : m_objectManager(objectManager), m_lightManager(lightManager), m_device(device),
    m_currentCam(nullptr), m_directionLight(nullptr)
{
}

void RenderManager::SetCamera(LPENTITY camera)
{
    m_currentCam = camera;
}

void RenderManager::SetDirectionalLight(LPENTITY dirLight)
{
    m_directionLight = dirLight;
}

void RenderManager::UpdateShadowMatrixBuffer(const DirectX::XMMATRIX& viewMatrix, const DirectX::XMMATRIX& projMatrix)
{
    if (!m_device.IsInitialized())
        return;

    ID3D11Buffer* shadowMatrixBuffer = m_device.GetShadowMatrixBuffer();
    if (!shadowMatrixBuffer)
        return;

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = m_device.GetDeviceContext()->Map(
        shadowMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

    if (FAILED(hr))
        return;

    struct ShadowMatrixBuffer {
        DirectX::XMMATRIX lightViewMatrix;
        DirectX::XMMATRIX lightProjectionMatrix;
    };

    // NOTE ABOUT TRANSPOSING:
    // Your VertexShader.hlsl declares the shadow matrices as 'row_major'.
    // With 'row_major' and mul(vector, matrix) usage, you should NOT transpose here.
    // If you later remove 'row_major' in HLSL or change multiply order, flip this.
    constexpr bool HLSL_USES_ROW_MAJOR = true;

    ShadowMatrixBuffer* bufferData = (ShadowMatrixBuffer*)mappedResource.pData;
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

    ID3D11DeviceContext* ctx = m_device.GetDeviceContext();
    ID3D11DepthStencilView* shadowDSV = m_device.GetShadowMapDepthView();
    if (!ctx || !shadowDSV)
        return;

    // ---- PASS 1 STATE (deterministisch) ----
    ctx->OMSetRenderTargets(0, nullptr, shadowDSV);

    // Hazard vermeiden (ShadowMap wird als SRV später gelesen)
    constexpr UINT SHADOW_TEX_SLOT = 7;
    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    ctx->PSSetShaderResources(SHADOW_TEX_SLOT, 1, nullSRV);
    ctx->VSSetShaderResources(SHADOW_TEX_SLOT, 1, nullSRV);

    ctx->ClearDepthStencilView(shadowDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

    if (ID3D11RasterizerState* rsShadow = m_device.GetShadowRasterState())
        ctx->RSSetState(rsShadow);

    UINT smW = 0, smH = 0;
    m_device.GetShadowMapSize(smW, smH);
    if (smW == 0 || smH == 0)
        return;

    D3D11_VIEWPORT vp{};
    vp.Width = (float)smW;
    vp.Height = (float)smH;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    ctx->RSSetViewports(1, &vp);

    // Depth-only
    ctx->PSSetShader(nullptr, nullptr, 0);

    // ---- Shadow Matrices updaten/binden (b3) ----
    Light* light = dynamic_cast<Light*>(m_directionLight);
    if (!light)
        return;

    const DirectX::XMMATRIX lightViewMatrix = light->GetLightViewMatrix();
    const DirectX::XMMATRIX lightProjMatrix = light->GetLightProjectionMatrix();

    UpdateShadowMatrixBuffer(lightViewMatrix, lightProjMatrix);

    if (ID3D11Buffer* shadowMatrixBuffer = m_device.GetShadowMatrixBuffer())
        ctx->VSSetConstantBuffers(3, 1, &shadowMatrixBuffer);

    // ---- Draw depth into shadow map ----
    for (Mesh* mesh : m_objectManager.GetMeshes())
    {
        if (!mesh || mesh->surfaces.empty()) continue;

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
    if (!ctx)
        return;

    // ---- PASS 2 STATE (deterministisch) ----
    ID3D11RenderTargetView* rtv = m_device.GetRenderTargetView();
    ID3D11DepthStencilView* dsv = m_device.GetDepthStencilView();
    if (!rtv || !dsv)
        return;

    ctx->OMSetRenderTargets(1, &rtv, dsv);

    if (ID3D11RasterizerState* rsDefault = m_device.GetRasterizerState())
        ctx->RSSetState(rsDefault);
    else
        ctx->RSSetState(nullptr);

    // Kamera-Viewport ist Pflicht (sonst “Shadow VP” bleibt aktiv)
    ctx->RSSetViewports(1, &m_currentCam->viewport);

    // ---- Shadow resources (t7/s7) – vorbereitet für späteren echten Shadow-PS ----
    constexpr UINT SHADOW_TEX_SLOT = 7;
    ID3D11ShaderResourceView* shadowSRV = m_device.GetShadowMapSRV();
    ID3D11SamplerState* shadowSampler = m_device.GetComparisonSampler();

    ctx->PSSetShaderResources(SHADOW_TEX_SLOT, 1, &shadowSRV);
    ctx->PSSetSamplers(SHADOW_TEX_SLOT, 1, &shadowSampler);

    // Optional, aber konsistent: Shadow-Matrixbuffer auch im Normalpass binden
    if (m_directionLight)
    {
        Light* light = dynamic_cast<Light*>(m_directionLight);
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

    for (Mesh* mesh : m_objectManager.GetMeshes())
    {
        if (!mesh || mesh->surfaces.empty()) continue;

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

    // ---- Queue-Inhalt einmalig loggen (nur wenn sich Größe ändert) ----
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
    for (ShaderBatch& sb : m_opaque.shaders)
    {
        if (!sb.shader) continue;
        sb.shader->UpdateShader(&m_device);

        for (MaterialBatch& mb : sb.materials)
        {
            if (!mb.material) continue;
            mb.material->SetTexture(&m_device);
            mb.material->UpdateConstantBuffer(m_device.GetDeviceContext());

            for (DrawEntry& entry : mb.draws)
            {
                if (!entry.mesh || !entry.surface) continue;

                // World-Matrix dieses Mesh in b0 laden
                entry.mesh->Update(&m_device, &m_currentCam->matrixSet);

                entry.surface->Draw(&m_device, sb.flagsVertex);
            }
        }
    }
}

void RenderManager::RenderScene()
{
    if (!m_currentCam)
        return;

    // PASS 1: Shadow Map befüllen
    RenderShadowPass();

    // PASS 2: Render Target + State vorbereiten
    RenderNormalPass();

    // Lights → b1
    m_lightManager.Update(&m_device);

    Debug::LogOnce("RenderScene_Camera",
        "Camera: ", Ptr(m_currentCam).c_str());

    // Phase 1: Szene traversieren, Queue nach Shader/Material gruppieren
    BuildRenderQueue();

    // Phase 2: Queue abarbeiten – minimale State-Changes
    FlushRenderQueue();
}