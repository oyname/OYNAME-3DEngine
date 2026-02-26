#pragma once
#include <DirectXMath.h>

class GDXDevice;
class ShadowMapTarget;
class BackbufferTarget;
class RenderTextureTarget;
struct D3D11_VIEWPORT;

// API-neutrales Backend-Interface.
// Schritt 1: Shadow-Matrix-CB (VS b3) + Shadow SRV/Sampler (PS t7/s7 bzw. SHADOW_TEX_SLOT) kapseln.
// Schritt 2: RenderTargets (Bind/Clear/Viewport) kapseln.
// WICHTIG: keine Behaviour-Aenderung, Slots und Reihenfolge bleiben exakt wie zuvor.
class IRenderBackend
{
public:
    virtual ~IRenderBackend() = default;

    // Step 1
    virtual void UpdateShadowMatrixBuffer(
        GDXDevice& device,
        const DirectX::XMMATRIX& lightViewMatrix,
        const DirectX::XMMATRIX& lightProjMatrix) = 0;

    virtual void BindShadowMatrixConstantBufferVS(GDXDevice& device) = 0;

    virtual void BindShadowResourcesPS(GDXDevice& device, ShadowMapTarget& shadowTarget) = 0;

    // Step 2
    virtual void BeginShadowPass(GDXDevice& device, ShadowMapTarget& shadowTarget) = 0;

    virtual void BeginMainPass(
        GDXDevice& device,
        BackbufferTarget& backbufferTarget,
        const D3D11_VIEWPORT& cameraViewport) = 0;

    virtual void BeginRttPass(GDXDevice& device, RenderTextureTarget& rttTarget) = 0;
};
