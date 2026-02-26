#pragma once
#include "IRenderBackend.h"

// DX11-Backend (copy + redirect).
// Enthält die ehemals direkten DX11-Calls aus RenderManager.cpp.
// Keine Slot-Aenderung, keine Reihenfolge-Aenderung.
class Dx11RenderBackend : public IRenderBackend
{
public:
    Dx11RenderBackend() = default;
    ~Dx11RenderBackend() override = default;

    void BindEntityConstants(GDXDevice& device, const Entity& entity) override;

    // Step 1
    void UpdateShadowMatrixBuffer(
        GDXDevice& device,
        const DirectX::XMMATRIX& lightViewMatrix,
        const DirectX::XMMATRIX& lightProjMatrix) override;

    void BindShadowMatrixConstantBufferVS(GDXDevice& device) override;

    void BindShadowResourcesPS(GDXDevice& device, ShadowMapTarget& shadowTarget) override;

    // Step 2
    void BeginShadowPass(GDXDevice& device, ShadowMapTarget& shadowTarget) override;

    void BeginMainPass(
        GDXDevice& device,
        BackbufferTarget& backbufferTarget,
        const Viewport& cameraViewport) override;

    void BeginRttPass(GDXDevice& device, RenderTextureTarget& rttTarget) override;
};
