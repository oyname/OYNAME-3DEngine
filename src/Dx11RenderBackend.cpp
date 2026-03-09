#include "Dx11RenderBackend.h"
#include "Dx11ShadowMap.h"
#include "Dx11LightManagerGpuData.h"
#include "LightArrayBuffer.h"
#include "gdxdevice.h"
#include "Entity.h"
#include "Mesh.h"
#include "Light.h"
#include "Dx11EntityGpuData.h"
#include "Dx11MaterialGpuData.h"
#include "Material.h"
#include "TexturePool.h"
#include "ShadowMapTarget.h"
#include "BackbufferTarget.h"
#include "RenderTextureTarget.h"

#include <d3d11.h>
#include <cstring>

#ifndef SHADOW_TEX_SLOT
#define SHADOW_TEX_SLOT 16   // must match PixelShader.hlsl register(t16)
#endif

#ifndef SHADOW_SMP_SLOT
#define SHADOW_SMP_SLOT 7    // must match PixelShader.hlsl register(s7)
#endif

Dx11RenderBackend::Dx11RenderBackend(GDXDevice& device)
{
    m_device = &device;
    device.AttachDx11Backend(this);
    m_shadow      = std::make_unique<Dx11ShadowMap>();
    m_lightGpuData = std::make_unique<Dx11LightManagerGpuData>();
    m_lightCBData  = std::make_unique<LightArrayBuffer>();

    CreateFrameStates(device);

    DBLOG("Dx11RenderBackend.cpp: Backend created");
}

Dx11RenderBackend::~Dx11RenderBackend()
{
    if (m_shadow) m_shadow->Release();

    if (m_defaultSampler)  { m_defaultSampler->Release();  m_defaultSampler  = nullptr; }
    if (m_alphaBlendState) { m_alphaBlendState->Release(); m_alphaBlendState = nullptr; }
    if (m_noBlendState)    { m_noBlendState->Release();    m_noBlendState    = nullptr; }
}

void Dx11RenderBackend::CreateFrameStates(GDXDevice& device)
{
    ID3D11Device* dev = device.GetDevice();
    if (!dev) return;

    if (!m_defaultSampler)
    {
        D3D11_SAMPLER_DESC sd{};
        sd.Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sd.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
        sd.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
        sd.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
        sd.MaxAnisotropy  = 1;
        sd.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        sd.MaxLOD         = D3D11_FLOAT32_MAX;
        HRESULT hr = dev->CreateSamplerState(&sd, &m_defaultSampler);
        if (SUCCEEDED(hr))
            DBLOG("Dx11RenderBackend.cpp: Default sampler for s0 created");
        else
            DBERROR("Dx11RenderBackend.cpp: Failed to create default sampler");
    }

    if (!m_alphaBlendState)
    {
        D3D11_BLEND_DESC bd{};
        bd.RenderTarget[0].BlendEnable           = TRUE;
        bd.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
        bd.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
        bd.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
        bd.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
        bd.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ZERO;
        bd.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
        bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        HRESULT hr = dev->CreateBlendState(&bd, &m_alphaBlendState);
        if (SUCCEEDED(hr))
            DBLOG("Dx11RenderBackend.cpp: Alpha blend state created");
        else
            DBERROR("Dx11RenderBackend.cpp: Failed to create alpha blend state");
    }

    if (!m_noBlendState)
    {
        D3D11_BLEND_DESC bd{};
        bd.RenderTarget[0].BlendEnable           = FALSE;
        bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        HRESULT hr = dev->CreateBlendState(&bd, &m_noBlendState);
        if (SUCCEEDED(hr))
            DBLOG("Dx11RenderBackend.cpp: No-blend state created");
        else
            DBERROR("Dx11RenderBackend.cpp: Failed to create no-blend state");
    }
}

Dx11ShadowMap& Dx11RenderBackend::GetShadow()
{
    return *m_shadow;
}

const Dx11ShadowMap& Dx11RenderBackend::GetShadow() const
{
    return *m_shadow;
}

bool Dx11RenderBackend::EnsureShadowCreated(GDXDevice& device, unsigned int width, unsigned int height)
{
    if (!m_shadow) m_shadow = std::make_unique<Dx11ShadowMap>();

    const unsigned int size = width; // keep behaviour: square shadow map
    (void)height;

    if (m_shadow->GetSize() == size && m_shadow->GetDSV() && m_shadow->GetSRV())
        return true;

    ID3D11Device* dev = device.GetDevice();
    if (!dev) return false;
    return m_shadow->Create(dev, size);
}

void Dx11RenderBackend::BindEntityConstants(GDXDevice& device, const Entity& entity)
{
    if (!device.IsInitialized()) return;
    if (!entity.gpuData) return;
    entity.gpuData->Bind(&device);
}

void Dx11RenderBackend::BindBoneConstants(GDXDevice& device, const Mesh& mesh)
{
    if (!mesh.hasSkinning || !mesh.boneConstantBuffer) return;
    if (!device.IsInitialized()) return;

    ID3D11DeviceContext* ctx = device.GetDeviceContext();
    if (!ctx) return;

    // Bone palette lives at VS b4. Slot is fixed; no PS bind needed.
    ctx->VSSetConstantBuffers(4, 1, &mesh.boneConstantBuffer);
}

// ---------------------------------------------------------------------------
// Step 5: light constants + entity frame stats
// ---------------------------------------------------------------------------

void Dx11RenderBackend::UploadLightConstants(
    const std::vector<Light*>& lights,
    const DirectX::XMFLOAT4&  globalAmbient)
{
    if (!m_device) return;

    if (!m_lightGpuData->IsReady())
        m_lightGpuData->Init(m_device, sizeof(LightArrayBuffer));

    for (size_t i = 0; i < lights.size(); ++i)
    {
        lights[i]->Update(m_device);

        // Only the first light carries the global ambient; the rest get zero.
        const DirectX::XMFLOAT4 ambient = (i == 0)
            ? globalAmbient
            : DirectX::XMFLOAT4(0.f, 0.f, 0.f, 0.f);

        m_lightCBData->lights[i].lightPosition      = lights[i]->cbLight.lightPosition;
        m_lightCBData->lights[i].lightDirection     = lights[i]->cbLight.lightDirection;
        m_lightCBData->lights[i].lightDiffuseColor  = lights[i]->cbLight.lightDiffuseColor;
        m_lightCBData->lights[i].lightAmbientColor  = ambient;
    }

    m_lightCBData->lightCount = static_cast<unsigned int>(lights.size());
    m_lightGpuData->Upload(m_device, *m_lightCBData);
}

IRenderBackend::EntityStats Dx11RenderBackend::GetEntityFrameStats() const
{
    const EntityGpuData::FrameStats s = EntityGpuData::GetFrameStats();
    return EntityStats{ s.uploads, s.binds, s.ringRotations };
}

void Dx11RenderBackend::ResetEntityFrameStats()
{
    EntityGpuData::ResetFrameStats();
}

void Dx11RenderBackend::UpdateShadowMatrixBuffer(
    GDXDevice& device,
    const DirectX::XMMATRIX& lightViewMatrix,
    const DirectX::XMMATRIX& lightProjMatrix)
{
    if (!device.IsInitialized()) return;

    ID3D11Buffer* shadowMatrixBuffer = device.GetShadowMatrixBuffer();
    if (!shadowMatrixBuffer) return;

    D3D11_MAPPED_SUBRESOURCE mappedResource{};
    HRESULT hr = device.GetDeviceContext()->Map(
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
        bufferData->lightViewMatrix       = lightViewMatrix;
        bufferData->lightProjectionMatrix = lightProjMatrix;
    }
    else
    {
        bufferData->lightViewMatrix       = DirectX::XMMatrixTranspose(lightViewMatrix);
        bufferData->lightProjectionMatrix = DirectX::XMMatrixTranspose(lightProjMatrix);
    }

    device.GetDeviceContext()->Unmap(shadowMatrixBuffer, 0);
}

void Dx11RenderBackend::BindShadowMatrixConstantBufferVS(GDXDevice& device)
{
    if (!device.IsInitialized()) return;

    if (ID3D11Buffer* shadowMatrixBuffer = device.GetShadowMatrixBuffer())
        device.GetDeviceContext()->VSSetConstantBuffers(3, 1, &shadowMatrixBuffer);
}

void Dx11RenderBackend::BindShadowResourcesPS(GDXDevice& device, ShadowMapTarget& shadowTarget)
{
    if (!device.IsInitialized()) return;

    (void)shadowTarget;

    ID3D11DeviceContext* ctx = device.GetDeviceContext();
    if (!ctx) return;

    ID3D11ShaderResourceView* shadowSRV = device.GetShadowMapSRV();
    ID3D11SamplerState*       shadowSmp = device.GetComparisonSampler();

    ctx->PSSetShaderResources(SHADOW_TEX_SLOT, 1, &shadowSRV);
    ctx->PSSetSamplers(SHADOW_SMP_SLOT, 1, &shadowSmp);
}

void Dx11RenderBackend::BeginShadowPass()
{
    if (!m_device || !m_device->IsInitialized()) return;
    ID3D11DeviceContext* ctx = m_device->GetDeviceContext();
    if (!ctx) return;

    // Prevent SRV hazard: release shadow slot before using it as DSV
    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    ctx->PSSetShaderResources(SHADOW_TEX_SLOT, 1, nullSRV);
    ctx->VSSetShaderResources(SHADOW_TEX_SLOT, 1, nullSRV);

    // No pixel shader needed (depth-only pass)
    ctx->PSSetShader(nullptr, nullptr, 0);

    if (m_shadow) m_shadow->Begin(ctx);
}

void Dx11RenderBackend::EndShadowPass()
{
    if (!m_device || !m_device->IsInitialized()) return;
    ID3D11DeviceContext* ctx = m_device->GetDeviceContext();
    if (!ctx) return;

    ID3D11RenderTargetView* rtv = m_device->GetRenderTargetView();
    ID3D11DepthStencilView* dsv = m_device->GetDepthStencilView();
    if (rtv && dsv)
        ctx->OMSetRenderTargets(1, &rtv, dsv);

    UINT w = 0, h = 0;
    if (ID3D11Texture2D* bb = m_device->GetBackBuffer())
    {
        D3D11_TEXTURE2D_DESC desc{};
        bb->GetDesc(&desc);
        w = desc.Width;
        h = desc.Height;
    }
    if (w > 0 && h > 0)
    {
        D3D11_VIEWPORT vp{};
        vp.TopLeftX = 0.0f;
        vp.TopLeftY = 0.0f;
        vp.Width    = (float)w;
        vp.Height   = (float)h;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        ctx->RSSetViewports(1, &vp);
    }

    if (ID3D11RasterizerState* rs = m_device->GetRasterizerState())
        ctx->RSSetState(rs);
    else
        ctx->RSSetState(nullptr);
}

void Dx11RenderBackend::BeginShadowPass(GDXDevice& device, ShadowMapTarget& shadowTarget)
{
    (void)device;
    (void)shadowTarget;
    BeginShadowPass();
}

void Dx11RenderBackend::BeginMainPass(
    GDXDevice& device,
    BackbufferTarget& backbufferTarget,
    const Viewport& cameraViewport)
{
    (void)backbufferTarget;

    if (!device.IsInitialized()) return;
    ID3D11DeviceContext*    ctx = device.GetDeviceContext();
    ID3D11RenderTargetView* rtv = device.GetRenderTargetView();
    ID3D11DepthStencilView* dsv = device.GetDepthStencilView();
    if (!ctx || !rtv || !dsv) return;

    D3D11_VIEWPORT dxVp{};
    dxVp.TopLeftX = cameraViewport.x;
    dxVp.TopLeftY = cameraViewport.y;
    dxVp.Width    = cameraViewport.width;
    dxVp.Height   = cameraViewport.height;
    dxVp.MinDepth = cameraViewport.minDepth;
    dxVp.MaxDepth = cameraViewport.maxDepth;

    ctx->OMSetRenderTargets(1, &rtv, dsv);

    if (ID3D11RasterizerState* rsDefault = device.GetRasterizerState())
        ctx->RSSetState(rsDefault);
    else
        ctx->RSSetState(nullptr);

    ctx->RSSetViewports(1, &dxVp);
}

void Dx11RenderBackend::EndMainPass()
{
    // Hook for post-pass work (debug overlays, post-fx etc.)
}

void Dx11RenderBackend::BeginRttPass(GDXDevice& device, RenderTextureTarget& rttTarget)
{
    if (!device.IsInitialized()) return;
    ID3D11DeviceContext* ctx = device.GetDeviceContext();
    if (!ctx) return;

    ID3D11RenderTargetView* rtv = rttTarget.GetRTV();
    ID3D11DepthStencilView* dsv = rttTarget.GetDSV();
    if (!rtv || !dsv) return;

    // SRV hazard: release relevant slots before binding RTV
    ID3D11ShaderResourceView* nullSRVs[8] = { nullptr };
    ctx->PSSetShaderResources(0, 8, nullSRVs);

    ctx->OMSetRenderTargets(1, &rtv, dsv);

    ctx->ClearRenderTargetView(rtv, rttTarget.GetClearColor());
    ctx->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    if (ID3D11RasterizerState* rsDefault = device.GetRasterizerState())
        ctx->RSSetState(rsDefault);
    else
        ctx->RSSetState(nullptr);

    D3D11_VIEWPORT vp = rttTarget.GetViewport();
    ctx->RSSetViewports(1, &vp);
}

void Dx11RenderBackend::EndRttPass()
{
    if (!m_device || !m_device->IsInitialized()) return;
    ID3D11DeviceContext* ctx = m_device->GetDeviceContext();
    if (!ctx) return;

    ID3D11RenderTargetView* rtv = m_device->GetRenderTargetView();
    ID3D11DepthStencilView* dsv = m_device->GetDepthStencilView();
    if (rtv && dsv)
        ctx->OMSetRenderTargets(1, &rtv, dsv);

    UINT w = 0, h = 0;
    if (ID3D11Texture2D* bb = m_device->GetBackBuffer())
    {
        D3D11_TEXTURE2D_DESC desc{};
        bb->GetDesc(&desc);
        w = desc.Width;
        h = desc.Height;
    }
    if (w > 0 && h > 0)
    {
        D3D11_VIEWPORT vp{};
        vp.TopLeftX = 0.0f;
        vp.TopLeftY = 0.0f;
        vp.Width    = (float)w;
        vp.Height   = (float)h;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        ctx->RSSetViewports(1, &vp);
    }

    if (ID3D11RasterizerState* rs = m_device->GetRasterizerState())
        ctx->RSSetState(rs);
    else
        ctx->RSSetState(nullptr);
}

// ---------------------------------------------------------------------------
// Step 3: material bind path
// ---------------------------------------------------------------------------

void Dx11RenderBackend::ResetMaterialCache()
{
    memset(m_boundSRVs, 0, sizeof(m_boundSRVs));
}

void Dx11RenderBackend::BindFrameSampler()
{
    if (!m_device || !m_defaultSampler) return;
    ID3D11DeviceContext* ctx = m_device->GetDeviceContext();
    if (ctx) ctx->PSSetSamplers(0, 1, &m_defaultSampler);
}

void Dx11RenderBackend::BindMaterial(const Material* material, const TexturePool* texturePool)
{
    if (!material || !m_device) return;
    ID3D11DeviceContext* ctx = m_device->GetDeviceContext();
    if (!ctx) return;

    if (texturePool)
    {
        // t0=Albedo  t1=Decal(2nd)  t2=Normal  t3=ORM
        // t4=Occlusion  t5=Roughness  t6=Metallic
        ID3D11ShaderResourceView* srvs[SRV_SLOT_COUNT] = {
            texturePool->GetSRV(material->albedoIndex),    // t0
            texturePool->GetSRV(material->decalIndex),     // t1
            texturePool->GetSRV(material->normalIndex),    // t2
            texturePool->GetSRV(material->ormIndex),       // t3
            texturePool->GetSRV(material->occlusionIndex), // t4
            texturePool->GetSRV(material->roughnessIndex), // t5
            texturePool->GetSRV(material->metallicIndex)   // t6
        };

        if (!srvs[0]) srvs[0] = texturePool->GetSRV(texturePool->WhiteIndex());
        if (!srvs[1]) srvs[1] = texturePool->GetSRV(texturePool->WhiteIndex());       // Decal: white = no influence
        if (!srvs[2]) srvs[2] = texturePool->GetSRV(texturePool->FlatNormalIndex());  // Normal flat fallback
        if (!srvs[3]) srvs[3] = texturePool->GetSRV(texturePool->OrmIndex());         // ORM fallback
        if (!srvs[4]) srvs[4] = texturePool->GetSRV(texturePool->WhiteIndex());
        if (!srvs[5]) srvs[5] = texturePool->GetSRV(texturePool->WhiteIndex());
        if (!srvs[6]) srvs[6] = texturePool->GetSRV(texturePool->WhiteIndex());

        if (memcmp(srvs, m_boundSRVs, sizeof(srvs)) != 0)
        {
            ctx->PSSetShaderResources(0, SRV_SLOT_COUNT, srvs);
            memcpy(m_boundSRVs, srvs, sizeof(srvs));
        }
    }
    else
    {
        if (material->gpuData) material->gpuData->SetTexture(m_device);
    }

    if (material->gpuData)
    {
        const auto& p = material->properties;
        material->gpuData->UpdateConstantBuffer(
            ctx,
            p.baseColor,
            p.specularColor,
            p.emissiveColor,
            p.uvTilingOffset,
            p.metallic,
            p.roughness,
            p.normalScale,
            p.occlusionStrength,
            p.shininess,
            p.transparency,
            p.alphaCutoff,
            p.receiveShadows,
            material->albedoIndex,
            material->normalIndex,
            material->ormIndex,
            material->decalIndex,
            p.blendMode,
            p.blendFactor,
            p.flags
        );
    }
}

void Dx11RenderBackend::SetAlphaBlend(bool enable)
{
    if (!m_device) return;
    ID3D11DeviceContext* ctx = m_device->GetDeviceContext();
    if (!ctx) return;

    const float blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
    if (enable && m_alphaBlendState)
        ctx->OMSetBlendState(m_alphaBlendState, blendFactor, 0xFFFFFFFF);
    else if (!enable && m_noBlendState)
        ctx->OMSetBlendState(m_noBlendState, blendFactor, 0xFFFFFFFF);
}
