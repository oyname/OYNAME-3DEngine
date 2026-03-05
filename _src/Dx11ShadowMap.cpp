#include "Dx11ShadowMap.h"

#include <d3d11.h>

#include "gdxutil.h" // Memory::SafeRelease, Debug::Log

Dx11ShadowMap::~Dx11ShadowMap()
{
    Release();
}

bool Dx11ShadowMap::Create(ID3D11Device* dev, UINT size)
{
    if (!dev || size == 0) return false;

    // Recreate safely
    Release();

    m_shadowSize = size;
    m_vpW = static_cast<float>(size);
    m_vpH = static_cast<float>(size);

    HRESULT hr = S_OK;

    // -------------------------
    // Shadow map texture (R32_TYPELESS)
    // -------------------------
    D3D11_TEXTURE2D_DESC shadowMapDesc{};
    shadowMapDesc.Width = size;
    shadowMapDesc.Height = size;
    shadowMapDesc.MipLevels = 1;
    shadowMapDesc.ArraySize = 1;
    shadowMapDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    shadowMapDesc.SampleDesc.Count = 1;
    shadowMapDesc.SampleDesc.Quality = 0;
    shadowMapDesc.Usage = D3D11_USAGE_DEFAULT;
    shadowMapDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

    hr = dev->CreateTexture2D(&shadowMapDesc, nullptr, &m_shadowTex);
    if (FAILED(hr))
    {
        Debug::LogError("Dx11ShadowMap: Failed to create shadow texture: ", hr);
        Release();
        return false;
    }

    // -------------------------
    // DSV (D32_FLOAT)
    // -------------------------
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    hr = dev->CreateDepthStencilView(m_shadowTex, &dsvDesc, &m_shadowDSV);
    if (FAILED(hr))
    {
        Debug::LogError("Dx11ShadowMap: Failed to create shadow DSV: ", hr);
        Release();
        return false;
    }

    // -------------------------
    // SRV (R32_FLOAT)
    // -------------------------
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;

    hr = dev->CreateShaderResourceView(m_shadowTex, &srvDesc, &m_shadowSRV);
    if (FAILED(hr))
    {
        Debug::LogError("Dx11ShadowMap: Failed to create shadow SRV: ", hr);
        Release();
        return false;
    }

    // -------------------------
    // Comparison sampler (PCF)
    // -------------------------
    D3D11_SAMPLER_DESC sampDesc{};
    sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.BorderColor[0] = 1.0f;
    sampDesc.BorderColor[1] = 1.0f;
    sampDesc.BorderColor[2] = 1.0f;
    sampDesc.BorderColor[3] = 1.0f;
    sampDesc.MinLOD = 0.0f;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    sampDesc.MipLODBias = 0.0f;
    sampDesc.MaxAnisotropy = 1;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;

    hr = dev->CreateSamplerState(&sampDesc, &m_shadowSampler);
    if (FAILED(hr))
    {
        Debug::LogError("Dx11ShadowMap: Failed to create comparison sampler: ", hr);
        Release();
        return false;
    }

    // -------------------------
    // Rasterizer state for shadow pass (front-face culling + slope bias)
    // -------------------------
    D3D11_RASTERIZER_DESC rsDesc{};
    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.CullMode = D3D11_CULL_FRONT;
    rsDesc.FrontCounterClockwise = FALSE;
    rsDesc.DepthBias = 0;
    rsDesc.DepthBiasClamp = 0.0f;
    rsDesc.SlopeScaledDepthBias = 1.5f;
    rsDesc.DepthClipEnable = TRUE;

    hr = dev->CreateRasterizerState(&rsDesc, &m_shadowRS);
    if (FAILED(hr))
    {
        Debug::LogError("Dx11ShadowMap: Failed to create shadow rasterizer state: ", hr);
        Release();
        return false;
    }

    // -------------------------
    // Shadow matrix constant buffer (b3): 2 matrices = 128 bytes
    // -------------------------
    D3D11_BUFFER_DESC bd{};
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = 128;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = dev->CreateBuffer(&bd, nullptr, &m_shadowMatrixCB);
    if (FAILED(hr))
    {
        Debug::LogError("Dx11ShadowMap: Failed to create shadow matrix CB: ", hr);
        Release();
        return false;
    }

    Debug::Log("Dx11ShadowMap: Created shadow resources (", size, "x", size, ")");
    return true;
}

void Dx11ShadowMap::Release()
{
    Memory::SafeRelease(m_shadowMatrixCB);
    Memory::SafeRelease(m_shadowRS);
    Memory::SafeRelease(m_shadowSampler);
    Memory::SafeRelease(m_shadowSRV);
    Memory::SafeRelease(m_shadowDSV);
    Memory::SafeRelease(m_shadowTex);

    m_shadowSize = 0;
    m_vpW = 0.0f;
    m_vpH = 0.0f;
}

void Dx11ShadowMap::Begin(ID3D11DeviceContext* ctx)
{
    if (!ctx || !m_shadowDSV || m_shadowSize == 0)
        return;

    // Depth-only: no color RTV
    ctx->OMSetRenderTargets(0, nullptr, m_shadowDSV);
    ctx->ClearDepthStencilView(m_shadowDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

    if (m_shadowRS)
        ctx->RSSetState(m_shadowRS);

    D3D11_VIEWPORT vp{};
    vp.Width = m_vpW;
    vp.Height = m_vpH;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    ctx->RSSetViewports(1, &vp);
}

void Dx11ShadowMap::End(ID3D11DeviceContext* ctx)
{
    (void)ctx;
    // Intentionally empty: state restore is handled by the backend.
}
