#include "Dx11RenderBackend.h"
#include "Dx11ShadowMap.h"
#include "gdxdevice.h"
#include "Entity.h"
#include "ShadowMapTarget.h"
#include "BackbufferTarget.h"
#include "RenderTextureTarget.h"

#include <d3d11.h>

#ifndef SHADOW_TEX_SLOT
#define SHADOW_TEX_SLOT 7
#endif

Dx11RenderBackend::Dx11RenderBackend(GDXDevice& device)
{
    m_device = &device;
    device.AttachDx11Backend(this);
    m_shadow = std::make_unique<Dx11ShadowMap>();
    
    if (ID3D11Device* dev = device.GetDevice())
        m_texturePool.InitializeDefaults(dev);

    Debug::Log("Texture Pool Size: ", m_texturePool.Size());
}

Dx11RenderBackend::~Dx11RenderBackend()
{
    if (m_shadow) m_shadow->Release();
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

    ID3D11Buffer* cb = entity.constantBuffer;
    if (!cb) return;

    ID3D11DeviceContext* ctx = device.GetDeviceContext();
    if (!ctx) return;

    ctx->VSSetConstantBuffers(0, 1, &cb);
    ctx->PSSetConstantBuffers(0, 1, &cb);
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

    // Original: constexpr bool HLSL_USES_ROW_MAJOR = true;
    constexpr bool HLSL_USES_ROW_MAJOR = true;

    ShadowMatrixBuffer* bufferData = reinterpret_cast<ShadowMatrixBuffer*>(mappedResource.pData);
    if (HLSL_USES_ROW_MAJOR)
    {
        bufferData->lightViewMatrix = lightViewMatrix;
        bufferData->lightProjectionMatrix = lightProjMatrix;
    }
    else
    {
        bufferData->lightViewMatrix = DirectX::XMMatrixTranspose(lightViewMatrix);
        bufferData->lightProjectionMatrix = DirectX::XMMatrixTranspose(lightProjMatrix);
    }

    device.GetDeviceContext()->Unmap(shadowMatrixBuffer, 0);
}

void Dx11RenderBackend::BindShadowMatrixConstantBufferVS(GDXDevice& device)
{
    if (!device.IsInitialized()) return;

    if (ID3D11Buffer* shadowMatrixBuffer = device.GetShadowMatrixBuffer())
        device.GetDeviceContext()->VSSetConstantBuffers(3, 1, &shadowMatrixBuffer); // Slot 3 unverändert
}

void Dx11RenderBackend::BindShadowResourcesPS(GDXDevice& device, ShadowMapTarget& shadowTarget)
{
    if (!device.IsInitialized()) return;

    (void)shadowTarget; // aktuell nur als "Handle"/Tag

    ID3D11DeviceContext* ctx = device.GetDeviceContext();
    if (!ctx) return;

    ID3D11ShaderResourceView* shadowSRV = device.GetShadowMapSRV();
    ID3D11SamplerState* shadowSmp = device.GetComparisonSampler();

    // Slot unverändert: SHADOW_TEX_SLOT (im Originalcode 7)
    ctx->PSSetShaderResources(SHADOW_TEX_SLOT, 1, &shadowSRV);
    ctx->PSSetSamplers(SHADOW_TEX_SLOT, 1, &shadowSmp);
}

void Dx11RenderBackend::BeginShadowPass()
{
    if (!m_device || !m_device->IsInitialized()) return;
    ID3D11DeviceContext* ctx = m_device->GetDeviceContext();
    if (!ctx) return;

    // SRV-Hazard verhindern: Shadow-Slot freigeben bevor als DSV genutzt
    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    ctx->PSSetShaderResources(SHADOW_TEX_SLOT, 1, nullSRV);
    ctx->VSSetShaderResources(SHADOW_TEX_SLOT, 1, nullSRV);

    // Kein Pixel-Shader nötig (Depth-only Pass)
    ctx->PSSetShader(nullptr, nullptr, 0);

    // Shadow bind/clear/viewport/RS in one place.
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

    // Viewport zurück auf Backbuffer
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

    // Standard RS wieder setzen
    if (ID3D11RasterizerState* rs = m_device->GetRasterizerState())
        ctx->RSSetState(rs);
    else
        ctx->RSSetState(nullptr);
}

// Legacy (kept)
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
    // copy + redirect: exakt der bisherige Ablauf, nur zentral im Backend
    (void)backbufferTarget;

    if (!device.IsInitialized()) return;
    ID3D11DeviceContext* ctx = device.GetDeviceContext();
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

    // Backbuffer binden
    ctx->OMSetRenderTargets(1, &rtv, dsv);

    // Pipeline-State fuer Main-Pass (vorher in BackbufferTarget::Bind)
    if (ID3D11RasterizerState* rsDefault = device.GetRasterizerState())
        ctx->RSSetState(rsDefault);
    else
        ctx->RSSetState(nullptr);

    // Kamera-Viewport (vorher in BackbufferTarget::Bind)
    ctx->RSSetViewports(1, &dxVp);
}

void Dx11RenderBackend::EndMainPass()
{
    // Intentionally minimal: the main pass usually targets the backbuffer and is the final pass.
    // Keeping this as a hook enables RenderManager to enforce an "atomic" pass boundary.
    // If you later add debug overlays / postfx, you can restore defaults here.
}

void Dx11RenderBackend::BeginRttPass(GDXDevice& device, RenderTextureTarget& rttTarget)
{
    if (!device.IsInitialized()) return;
    ID3D11DeviceContext* ctx = device.GetDeviceContext();
    if (!ctx) return;

    ID3D11RenderTargetView* rtv = rttTarget.GetRTV();
    ID3D11DepthStencilView* dsv = rttTarget.GetDSV();
    if (!rtv || !dsv) return;

    // SRV-Hazard: relevante Slots freigeben bevor RTV gebunden wird.
    // copy+redirect: entspricht dem bisherigen Reset in RenderTextureTarget::Bind()
    ID3D11ShaderResourceView* nullSRVs[8] = { nullptr };
    ctx->PSSetShaderResources(0, 8, nullSRVs);

    ctx->OMSetRenderTargets(1, &rtv, dsv);

    // Clear
    ctx->ClearRenderTargetView(rtv, rttTarget.GetClearColor());
    ctx->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // Pipeline-State fuer RTT-Pass (vorher in RenderTextureTarget::Bind)
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

    // Restore backbuffer RT/DS
    ID3D11RenderTargetView* rtv = m_device->GetRenderTargetView();
    ID3D11DepthStencilView* dsv = m_device->GetDepthStencilView();
    if (rtv && dsv)
        ctx->OMSetRenderTargets(1, &rtv, dsv);

    // Restore full backbuffer viewport (same behaviour as EndShadowPass)
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

    // Restore default RS
    if (ID3D11RasterizerState* rs = m_device->GetRasterizerState())
        ctx->RSSetState(rs);
    else
        ctx->RSSetState(nullptr);
}
