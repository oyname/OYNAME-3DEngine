#pragma once
#include "IRenderBackend.h"
#include "TexturePool.h"
#include "Dx11ShadowMap.h"

#include <memory>

struct ID3D11SamplerState;
struct ID3D11BlendState;
struct ID3D11ShaderResourceView;

class Dx11ShadowMap;
class GDXDevice;

// DX11 backend (copy + redirect).
// Houses all DX11 state that was previously scattered in RenderManager:
//   - default sampler (s0)
//   - alpha blend state / no-blend state
//   - SRV cache for t0..t6
// These are created in the constructor once the device is available.
class Dx11RenderBackend : public IRenderBackend
{
public:
    TexturePool m_texturePool;

public:
    explicit Dx11RenderBackend(GDXDevice& device);
    ~Dx11RenderBackend() override;

    void BindEntityConstants(GDXDevice& device, const Entity& entity) override;

    // Step 1
    void UpdateShadowMatrixBuffer(
        GDXDevice& device,
        const DirectX::XMMATRIX& lightViewMatrix,
        const DirectX::XMMATRIX& lightProjMatrix) override;

    void BindShadowMatrixConstantBufferVS(GDXDevice& device) override;

    void BindShadowResourcesPS(GDXDevice& device, ShadowMapTarget& shadowTarget) override;

    // Step 2
    void BeginShadowPass() override;
    void EndShadowPass() override;

    // Legacy (kept)
    void BeginShadowPass(GDXDevice& device, ShadowMapTarget& shadowTarget) override;

    void BeginMainPass(
        GDXDevice& device,
        BackbufferTarget& backbufferTarget,
        const Viewport& cameraViewport) override;

    void EndMainPass() override;

    void BeginRttPass(GDXDevice& device, RenderTextureTarget& rttTarget) override;

    void EndRttPass() override;

    // Step 3
    void ResetMaterialCache() override;
    void BindFrameSampler() override;
    void BindMaterial(const Material* material, const TexturePool* texturePool) override;
    void SetAlphaBlend(bool enable) override;

    // Internal: called by GDXDevice::CreateShadowBuffer (API-stable wrapper).
    bool EnsureShadowCreated(GDXDevice& device, unsigned int width, unsigned int height);

    Dx11ShadowMap& GetShadow();
    const Dx11ShadowMap& GetShadow() const;

private:
    GDXDevice* m_device = nullptr;

    std::unique_ptr<Dx11ShadowMap> m_shadow;

    // Frame-level DX11 state (formerly in RenderManager)
    ID3D11SamplerState* m_defaultSampler  = nullptr;
    ID3D11BlendState*   m_alphaBlendState = nullptr;
    ID3D11BlendState*   m_noBlendState    = nullptr;

    // SRV cache: last-bound set for t0..t6 (7 slots).
    // Avoids redundant PSSetShaderResources calls between materials.
    static constexpr int SRV_SLOT_COUNT = 7;
    ID3D11ShaderResourceView* m_boundSRVs[SRV_SLOT_COUNT] = {};

    void CreateFrameStates(GDXDevice& device);
};
