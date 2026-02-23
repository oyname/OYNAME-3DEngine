#include "ShadowMapTarget.h"
#include "gdxdevice.h"
#include "gdxutil.h"

void ShadowMapTarget::Bind()
{
    if (!m_device) return;

    ID3D11DeviceContext* ctx = m_device->GetDeviceContext();
    ID3D11DepthStencilView* shadowDSV = m_device->GetShadowMapDepthView();
    if (!ctx || !shadowDSV) return;

    // SRV-Hazard verhindern: Shadow-Slot freigeben bevor als DSV genutzt
    UnbindFromShader(7);

    // Depth-only: kein Color-RTV
    ctx->OMSetRenderTargets(0, nullptr, shadowDSV);

    // Shadow-Rasterizer (Front-Face-Culling, Depth Bias)
    if (ID3D11RasterizerState* rsShadow = m_device->GetShadowRasterState())
        ctx->RSSetState(rsShadow);

    // Shadow-Map-Viewport
    UINT smW = 0, smH = 0;
    m_device->GetShadowMapSize(smW, smH);
    if (smW == 0 || smH == 0)
    {
        Debug::LogError("ShadowMapTarget.cpp: Bind() – Shadow Map hat keine gueltige Groesse");
        return;
    }

    D3D11_VIEWPORT vp{};
    vp.Width    = static_cast<float>(smW);
    vp.Height   = static_cast<float>(smH);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    ctx->RSSetViewports(1, &vp);

    // Kein Pixel-Shader nötig (Depth-only Pass)
    ctx->PSSetShader(nullptr, nullptr, 0);

    Debug::LogOnce("ShadowMapTarget_Bind",
        "ShadowMapTarget.cpp: Bind() – Shadow Pass aktiv (", smW, "x", smH, ")");
}

void ShadowMapTarget::Clear()
{
    if (!m_device) return;

    ID3D11DeviceContext* ctx = m_device->GetDeviceContext();
    ID3D11DepthStencilView* shadowDSV = m_device->GetShadowMapDepthView();
    if (!ctx || !shadowDSV) return;

    ctx->ClearDepthStencilView(shadowDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void ShadowMapTarget::UnbindFromShader(unsigned int slot)
{
    if (!m_device) return;

    ID3D11DeviceContext* ctx = m_device->GetDeviceContext();
    if (!ctx) return;

    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    ctx->PSSetShaderResources(slot, 1, nullSRV);
    ctx->VSSetShaderResources(slot, 1, nullSRV);
}

ID3D11ShaderResourceView* ShadowMapTarget::GetSRV() const
{
    if (!m_device) return nullptr;
    return m_device->GetShadowMapSRV();
}
