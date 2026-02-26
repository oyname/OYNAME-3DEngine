#include "Dx11RenderBackend.h"
#include "gdxdevice.h"
#include "ShadowMapTarget.h"
#include "BackbufferTarget.h"
#include "RenderTextureTarget.h"

#include <d3d11.h>

#ifndef SHADOW_TEX_SLOT
#define SHADOW_TEX_SLOT 7
#endif

void Dx11RenderBackend::UpdateShadowMatrixBuffer(
    GDXDevice& device,
    const DirectX::XMMATRIX& lightViewMatrix,
    const DirectX::XMMATRIX& lightProjMatrix)
{
    if (!device.IsInitialized()) return;

    ID3D11Buffer* shadowMatrixBuffer = device.GetShadowMatrixBuffer();
    if (!shadowMatrixBuffer) return;

    D3D11_MAPPED_SUBRESOURCE mappedResource{};
    HRESULT hr = device.GetDeviceContext()->Map(
        shadowMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(hr)) return;

    struct ShadowMatrixBuffer
    {
        DirectX::XMMATRIX lightViewMatrix;
        DirectX::XMMATRIX lightProjectionMatrix;
    };

    // Original: constexpr bool HLSL_USES_ROW_MAJOR = true;
    // -> keine Transpose-Änderung!
    constexpr bool HLSL_USES_ROW_MAJOR = true;

    ShadowMatrixBuffer* bufferData = reinterpret_cast<ShadowMatrixBuffer*>(mappedResource.pData);
    if (HLSL_USES_ROW_MAJOR)
    {
        bufferData->lightViewMatrix = lightViewMatrix;
        bufferData->lightProjectionMatrix = lightProjMatrix;
    }
    else
    {
        bufferData->lightViewMatrix = DirectX::XMMatrixTranspose(lightViewMatrix);
        bufferData->lightProjectionMatrix = DirectX::XMMatrixTranspose(lightProjMatrix);
    }

    device.GetDeviceContext()->Unmap(shadowMatrixBuffer, 0);
}

void Dx11RenderBackend::BindShadowMatrixConstantBufferVS(GDXDevice& device)
{
    if (!device.IsInitialized()) return;

    if (ID3D11Buffer* shadowMatrixBuffer = device.GetShadowMatrixBuffer())
        device.GetDeviceContext()->VSSetConstantBuffers(3, 1, &shadowMatrixBuffer); // Slot 3 unverändert
}

void Dx11RenderBackend::BindShadowResourcesPS(GDXDevice& device, ShadowMapTarget& shadowTarget)
{
    if (!device.IsInitialized()) return;

    ID3D11DeviceContext* ctx = device.GetDeviceContext();
    if (!ctx) return;

    ID3D11ShaderResourceView* shadowSRV = shadowTarget.GetSRV();
    ID3D11SamplerState* shadowSmp = device.GetComparisonSampler();

    // Slot unverändert: SHADOW_TEX_SLOT (im Originalcode 7)
    ctx->PSSetShaderResources(SHADOW_TEX_SLOT, 1, &shadowSRV);
    ctx->PSSetSamplers(SHADOW_TEX_SLOT, 1, &shadowSmp);
}


// -----------------------------
// Schritt 2: RenderTargets (Bind/Clear/Viewport) - copy + redirect
// -----------------------------

void Dx11RenderBackend::BeginShadowPass(GDXDevice& device, ShadowMapTarget& shadowTarget)
{
    (void)device;
    shadowTarget.Bind();
    shadowTarget.Clear();
}

void Dx11RenderBackend::BeginMainPass(
    GDXDevice& device,
    BackbufferTarget& backbufferTarget,
    const D3D11_VIEWPORT& cameraViewport)
{
    (void)device;
    backbufferTarget.SetViewport(cameraViewport);
    backbufferTarget.Bind();
}

void Dx11RenderBackend::BeginRttPass(GDXDevice& device, RenderTextureTarget& rttTarget)
{
    (void)device;
    rttTarget.Bind();
    rttTarget.Clear();
}
