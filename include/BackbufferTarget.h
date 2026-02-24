#pragma once
#include "IRenderTarget.h"
#include <d3d11.h>

class GDXDevice;

class BackbufferTarget : public IRenderTarget
{
public:
    void SetDevice(GDXDevice* device) { m_device = device; }

    // Kamera-Viewport muss vor Bind() gesetzt werden
    void SetViewport(const D3D11_VIEWPORT& vp) { m_viewport = vp; }

    // Clearfarbe setzen (RGBA, Default: schwarz)
    void SetClearColor(float r, float g, float b, float a = 1.0f);

    // IRenderTarget
    void Bind()  override;
    void Clear() override;

    // Direktaufruf mit expliziter Farbe (Rückwärtskompatibilität)
    void ClearColorDepth(const float rgba[4], float depth = 1.0f, unsigned char stencil = 0);

private:
    GDXDevice*     m_device   = nullptr; // non-owning
    D3D11_VIEWPORT m_viewport = {};
    float          m_clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
};
