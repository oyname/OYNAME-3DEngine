#pragma once
#include <DirectXMath.h>
#include <vector>
#include "Viewport.h"

class GDXDevice;
class Entity;
class Mesh;
class Light;
class Material;
class TexturePool;
class ShadowMapTarget;
class BackbufferTarget;
class RenderTextureTarget;

// API-neutral backend interface.
// Step 1: Shadow-Matrix-CB (VS b3) + Shadow SRV/Sampler (PS t16/s7) encapsulated.
// Step 2: RenderTargets (Bind/Clear/Viewport) encapsulated.
// Step 3: Material bind path (SRVs t0..t6 + CB b2) + frame-level states encapsulated.
// Step 4: Per-entity constant path fully encapsulated (matrix CB b0, bone CB b4).
// Step 5: Light constant upload (CB b1) + entity frame stats encapsulated.
// IMPORTANT: no behavior change, slots and order remain exactly as before.
class IRenderBackend
{
public:
    // Per-frame entity upload counters collected by EntityGpuData.
    // Exposed here so RenderManager never needs to include Dx11EntityGpuData.h.
    struct EntityStats
    {
        unsigned int uploads       = 0;
        unsigned int binds         = 0;
        unsigned int ringRotations = 0;
    };

    virtual ~IRenderBackend() = default;

    // Step 4 ----------------------------------------------------------------

    // Binds the entity matrix constant buffer to VS b0 and PS b0.
    // Called when the buffer is already current (upload happened earlier this frame).
    virtual void BindEntityConstants(GDXDevice& device, const Entity& entity) = 0;

    // Binds the bone palette constant buffer to VS b4.
    // No-op when the mesh has no skinning data.
    virtual void BindBoneConstants(GDXDevice& device, const Mesh& mesh) = 0;

    // Step 5 ----------------------------------------------------------------

    // Assembles and uploads the light array constant buffer to PS/VS b1.
    // Calls light->Update() internally to flush per-light GPU data before copy.
    // globalAmbient is applied to the first light's ambient slot.
    virtual void UploadLightConstants(
        const std::vector<Light*>& lights,
        const DirectX::XMFLOAT4&  globalAmbient) = 0;

    // Returns upload/bind/ring counters accumulated by EntityGpuData this frame.
    virtual EntityStats GetEntityFrameStats() const = 0;

    // Resets EntityGpuData frame counters. Call once at the start of each frame.
    virtual void ResetEntityFrameStats() = 0;

    // Step 1 ----------------------------------------------------------------

    virtual void UpdateShadowMatrixBuffer(
        GDXDevice& device,
        const DirectX::XMMATRIX& lightViewMatrix,
        const DirectX::XMMATRIX& lightProjMatrix) = 0;

    virtual void BindShadowMatrixConstantBufferVS(GDXDevice& device) = 0;

    virtual void BindShadowResourcesPS(GDXDevice& device, ShadowMapTarget& shadowTarget) = 0;

    // Step 2 ----------------------------------------------------------------

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

    // Step 3 ----------------------------------------------------------------

    // Resets the internal SRV cache. Call at the start of each render flush.
    virtual void ResetMaterialCache() = 0;

    // Binds the engine default linear-wrap sampler to PS slot s0.
    virtual void BindFrameSampler() = 0;

    // Binds material SRVs (t0..t6) and CB (b2).
    virtual void BindMaterial(const Material* material, const TexturePool* texturePool) = 0;

    // Enables or disables alpha blending on render target 0.
    virtual void SetAlphaBlend(bool enable) = 0;
};
