#pragma once
#include <DirectXMath.h>
#include "Viewport.h"

class GDXDevice;
class Entity;
class Material;
class TexturePool;
class ShadowMapTarget;
class BackbufferTarget;
class RenderTextureTarget;

// API-neutral backend interface.
// Step 1: Shadow-Matrix-CB (VS b3) + Shadow SRV/Sampler (PS t16/s7) encapsulated.
// Step 2: RenderTargets (Bind/Clear/Viewport) encapsulated.
// Step 3: Material bind path (SRVs t0..t6 + CB b2) + frame-level states encapsulated.
// IMPORTANT: no behavior change, slots and order remain exactly as before.
class IRenderBackend
{
public:
    virtual ~IRenderBackend() = default;

    // Mesh/Entity constants (VS/PS b0): encapsulates backend-specific binding.
    virtual void BindEntityConstants(GDXDevice& device, const Entity& entity) = 0;

    // Step 1
    virtual void UpdateShadowMatrixBuffer(
        GDXDevice& device,
        const DirectX::XMMATRIX& lightViewMatrix,
        const DirectX::XMMATRIX& lightProjMatrix) = 0;

    virtual void BindShadowMatrixConstantBufferVS(GDXDevice& device) = 0;

    virtual void BindShadowResourcesPS(GDXDevice& device, ShadowMapTarget& shadowTarget) = 0;

    // Step 2
    virtual void BeginShadowPass() = 0;
    virtual void EndShadowPass() = 0;

    // Legacy (kept for compatibility)
    virtual void BeginShadowPass(GDXDevice& device, ShadowMapTarget& shadowTarget)
    {
        (void)device; (void)shadowTarget;
        BeginShadowPass();
    }

    virtual void BeginMainPass(
        GDXDevice& device,
        BackbufferTarget& backbufferTarget,
        const Viewport& cameraViewport) = 0;

    virtual void EndMainPass() {}

    virtual void BeginRttPass(GDXDevice& device, RenderTextureTarget& rttTarget) = 0;

    virtual void EndRttPass() {}

    // Step 3: Material bind path + frame-level GPU state.

    // Resets the internal SRV cache. Call at the start of each render flush so
    // that stale SRVs from a previous pass (e.g. shadow or RTT) are not assumed
    // to still be bound.
    virtual void ResetMaterialCache() = 0;

    // Binds the engine default linear-wrap sampler to PS slot s0.
    // Called once per flush, before the draw loop.
    virtual void BindFrameSampler() = 0;

    // Binds material resources to the pixel shader:
    //   t0..t6  - SRVs looked up via TexturePool (with built-in fallbacks)
    //   b2      - material constant buffer (all Material::properties fields)
    // Internally caches the last-bound SRV set to skip redundant PSSetShaderResources calls.
    // When texturePool == nullptr, falls back to gpuData->SetTexture().
    virtual void BindMaterial(const Material* material, const TexturePool* texturePool) = 0;

    // Enables or disables alpha blending on render target 0.
    // true  = SRC_ALPHA / INV_SRC_ALPHA  (transparent pass)
    // false = blending off               (opaque pass / restore after transparent)
    virtual void SetAlphaBlend(bool enable) = 0;
};
