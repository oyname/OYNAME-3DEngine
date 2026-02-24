#include "BackbufferTarget.h"
#include "gdxdevice.h"
#include "gdxutil.h"

void BackbufferTarget::SetClearColor(float r, float g, float b, float a)
{
    m_clearColor[0] = r;
    m_clearColor[1] = g;
    m_clearColor[2] = b;
    m_clearColor[3] = a;
}

void BackbufferTarget::Bind()
{
    if (!m_device) return;

    ID3D11DeviceContext* ctx = m_device->GetDeviceContext();
    ID3D11RenderTargetView* rtv = m_device->GetRenderTargetView();
    ID3D11DepthStencilView* dsv = m_device->GetDepthStencilView();
    if (!ctx || !rtv || !dsv)
    {
        Debug::LogError("BackbufferTarget.cpp: Bind() RTV oder DSV nicht verfuegbar");
        return;
    }

    ctx->OMSetRenderTargets(1, &rtv, dsv);

    // Standard-Rasterizer (Back-Face-Culling)
    if (ID3D11RasterizerState* rsDefault = m_device->GetRasterizerState())
        ctx->RSSetState(rsDefault);
    else
        ctx->RSSetState(nullptr);

    // Kamera-Viewport setzen – PFLICHT nach Shadow-Pass (sonst bleibt Shadow-VP aktiv)
    ctx->RSSetViewports(1, &m_viewport);

    Debug::LogOnce("BackbufferTarget_Bind",
        "BackbufferTarget.cpp: Bind() Backbuffer aktiv");
}

void BackbufferTarget::Clear()
{
    ClearColorDepth(m_clearColor);
}

void BackbufferTarget::ClearColorDepth(const float rgba[4], float depth, unsigned char stencil)
{
    if (!m_device) return;

    ID3D11DeviceContext* ctx = m_device->GetDeviceContext();
    ID3D11RenderTargetView* rtv = m_device->GetRenderTargetView();
    ID3D11DepthStencilView* dsv = m_device->GetDepthStencilView();
    if (!ctx || !rtv || !dsv) return;

    ctx->ClearRenderTargetView(rtv, rgba);
    ctx->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil);
}
