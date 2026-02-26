#pragma once
#include <DirectXMath.h>
#include "Viewport.h"

class GDXDevice;
class Entity;
class ShadowMapTarget;
class BackbufferTarget;
class RenderTextureTarget;

// API-neutrales Backend-Interface.
// Schritt 1: Shadow-Matrix-CB (VS b3) + Shadow SRV/Sampler (PS t7/s7 bzw. SHADOW_TEX_SLOT) kapseln.
// Schritt 2: RenderTargets (Bind/Clear/Viewport) kapseln.
// WICHTIG: keine Behaviour-Aenderung, Slots und Reihenfolge bleiben exakt wie zuvor.
class IRenderBackend
{
public:
    virtual ~IRenderBackend() = default;

    // Mesh/Entity constants (VS/PS b0): kapselt backend-spezifisches Binden.
    // NOTE: Entity ist API-neutral; dass intern aktuell ein DX11-Buffer steckt,
    // bleibt in diesem Schritt bewusst unveraendert (copy + redirect).
    virtual void BindEntityConstants(GDXDevice& device, const Entity& entity) = 0;

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
        const Viewport& cameraViewport) = 0;

    virtual void BeginRttPass(GDXDevice& device, RenderTextureTarget& rttTarget) = 0;
};
