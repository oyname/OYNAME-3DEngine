#pragma once
#include "IRenderBackend.h"
#include "Dx11ShadowMap.h"   

#include <memory>

class Dx11ShadowMap;
class GDXDevice;

// DX11-Backend (copy + redirect).
// Enthält die ehemals direkten DX11-Calls aus RenderManager.cpp.
// Keine Slot-Aenderung, keine Reihenfolge-Aenderung.
class Dx11RenderBackend : public IRenderBackend
{
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

    // Internal: called by GDXDevice::CreateShadowBuffer (API-stable wrapper).
    bool EnsureShadowCreated(GDXDevice& device, unsigned int width, unsigned int height);

    Dx11ShadowMap& GetShadow();
    const Dx11ShadowMap& GetShadow() const;

private:
    GDXDevice* m_device = nullptr;

    std::unique_ptr<Dx11ShadowMap> m_shadow;
};
