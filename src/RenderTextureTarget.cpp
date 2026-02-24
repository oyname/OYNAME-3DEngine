#include "RenderTextureTarget.h"
#include "gdxdevice.h"
#include "gdxutil.h"

RenderTextureTarget::RenderTextureTarget()
{
    m_clearColor[0] = 0.0f;
    m_clearColor[1] = 0.0f;
    m_clearColor[2] = 0.0f;
    m_clearColor[3] = 1.0f;
}

RenderTextureTarget::~RenderTextureTarget()
{
    Release();
}

HRESULT RenderTextureTarget::Create(ID3D11Device* device, ID3D11DeviceContext* context,
    UINT width, UINT height)
{
    if (!device || !context || width == 0 || height == 0)
    {
        Debug::LogError("RenderTextureTarget.cpp: Create() – ungueltige Parameter");
        return E_INVALIDARG;
    }

    Release();

    m_device = device;
    m_context = context;
    m_width = width;
    m_height = height;

    HRESULT hr = S_OK;

    // ── 1. Color-Textur (RGBA, shader-lesbar) ──────────────────────────────
    {
        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

        hr = device->CreateTexture2D(&desc, nullptr, &m_rttTexture);
        if (FAILED(hr))
        {
            Debug::LogError("RenderTextureTarget.cpp: Create() – CreateTexture2D (Color) fehlgeschlagen");
            Release();
            return hr;
        }
    }

    // ── 2. Render Target View ──────────────────────────────────────────────
    {
        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

        hr = device->CreateRenderTargetView(m_rttTexture, &rtvDesc, &m_rttRTV);
        if (FAILED(hr))
        {
            Debug::LogError("RenderTextureTarget.cpp: Create() – CreateRenderTargetView fehlgeschlagen");
            Release();
            return hr;
        }
    }

    // ── 3. Shader Resource View ────────────────────────────────────────────
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Texture2D.MostDetailedMip = 0;

        hr = device->CreateShaderResourceView(m_rttTexture, &srvDesc, &m_rttSRV);
        if (FAILED(hr))
        {
            Debug::LogError("RenderTextureTarget.cpp: Create() – CreateShaderResourceView fehlgeschlagen");
            Release();
            return hr;
        }
    }

    // ── 4. Depth-Textur für den RTT-Pass ──────────────────────────────────
    {
        D3D11_TEXTURE2D_DESC depthDesc = {};
        depthDesc.Width = width;
        depthDesc.Height = height;
        depthDesc.MipLevels = 1;
        depthDesc.ArraySize = 1;
        depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthDesc.SampleDesc.Count = 1;
        depthDesc.Usage = D3D11_USAGE_DEFAULT;
        depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

        hr = device->CreateTexture2D(&depthDesc, nullptr, &m_depthTexture);
        if (FAILED(hr))
        {
            Debug::LogError("RenderTextureTarget.cpp: Create() – CreateTexture2D (Depth) fehlgeschlagen");
            Release();
            return hr;
        }
    }

    // ── 5. Depth Stencil View ─────────────────────────────────────────────
    {
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

        hr = device->CreateDepthStencilView(m_depthTexture, &dsvDesc, &m_depthView);
        if (FAILED(hr))
        {
            Debug::LogError("RenderTextureTarget.cpp: Create() – CreateDepthStencilView fehlgeschlagen");
            Release();
            return hr;
        }
    }

    // ── 6. Texture-Wrapper: SRV eintragen (non-owning – Release() gibt ihn NICHT frei) ──
    // Der Wrapper besitzt die Ressource NICHT; m_rttSRV wird separat verwaltet.
    m_textureWrapper.m_textureView = m_rttSRV;
    m_textureWrapper.m_texture = m_rttTexture;

    Debug::Log("RenderTextureTarget.cpp: Create() OK – ", width, "x", height,
        " (RTV + SRV + Depth)");
    return S_OK;
}

void RenderTextureTarget::Release()
{
    // Wrapper-Pointer nullen, BEVOR die echten Ressourcen freigegeben werden,
    // damit kein Dangling-Pointer in m_textureWrapper verbleibt.
    m_textureWrapper.m_textureView = nullptr;
    m_textureWrapper.m_texture = nullptr;

    Memory::SafeRelease(m_rttRTV);
    Memory::SafeRelease(m_rttSRV);
    Memory::SafeRelease(m_rttTexture);
    Memory::SafeRelease(m_depthView);
    Memory::SafeRelease(m_depthTexture);

    m_device = nullptr;
    m_context = nullptr;
    m_gdxDevice = nullptr;
    m_width = 0;
    m_height = 0;
}

// ── RenderTarget interface ────────────────────────────────────────────────────

void RenderTextureTarget::Bind()
{
    if (!m_context || !m_rttRTV || !m_depthView)
    {
        Debug::LogError("RenderTextureTarget.cpp: Bind() – Ressourcen nicht initialisiert");
        return;
    }

    // Alle relevanten SRV-Slots freigeben bevor RTV gebunden wird.
    // Slot 0 = Farbtextur, Slot 7 = Shadow Map SRV.
    // Ohne diesen Reset entsteht ein D3D11-SRV-Hazard und der Shadow SRV
    // kann danach in RenderNormalPass() nicht korrekt gebunden werden.
    ID3D11ShaderResourceView* nullSRVs[8] = { nullptr };
    m_context->PSSetShaderResources(0, 8, nullSRVs);

    m_context->OMSetRenderTargets(1, &m_rttRTV, m_depthView);

    // Rasterizer-Reset: Der Shadow-Pass setzt Front-Face-Culling (D3D11_CULL_FRONT).
    // Ohne Reset sieht man im RTT-Pass die Innenseite aller Meshes.
    // Bevorzugt: Engine-eigener Default-Rasterizer über GDXDevice.
    // Fallback: nullptr → D3D11-Garantierter Default (CULL_BACK, FILL_SOLID).
    if (m_gdxDevice)
    {
        ID3D11RasterizerState* rsDefault = m_gdxDevice->GetRasterizerState();
        m_context->RSSetState(rsDefault);  // nullptr ist hier ebenfalls gültig (D3D11 Default)
        Debug::LogOnce("RenderTextureTarget_RSReset",
            "RenderTextureTarget.cpp: Bind() – Rasterizer via GDXDevice zurueckgesetzt");
    }
    else
    {
        // RSSetState(nullptr) setzt D3D11 auf den garantierten Default zurück:
        // CullMode=BACK, FillMode=SOLID, FrontCounterClockwise=FALSE
        m_context->RSSetState(nullptr);
        Debug::LogOnce("RenderTextureTarget_RSResetFallback",
            "RenderTextureTarget.cpp: Bind() – Rasterizer via nullptr (D3D11 Default) zurueckgesetzt");
    }

    D3D11_VIEWPORT vp = GetViewport();
    m_context->RSSetViewports(1, &vp);

    Debug::LogOnce("RenderTextureTarget_Bind",
        "RenderTextureTarget.cpp: Bind() – RTT aktiv (", m_width, "x", m_height, ")");
}

void RenderTextureTarget::Clear()
{
    if (!m_context) return;

    if (m_rttRTV)
        m_context->ClearRenderTargetView(m_rttRTV, m_clearColor);

    if (m_depthView)
        m_context->ClearDepthStencilView(m_depthView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
            1.0f, 0);
}

void RenderTextureTarget::UnbindFromShader(unsigned int slot)
{
    if (!m_context) return;
    ID3D11ShaderResourceView* nullSRV = nullptr;
    m_context->PSSetShaderResources(slot, 1, &nullSRV);
}

ID3D11ShaderResourceView* RenderTextureTarget::GetSRV() const
{
    return m_rttSRV;
}

D3D11_VIEWPORT RenderTextureTarget::GetViewport() const
{
    D3D11_VIEWPORT vp{};
    vp.Width = static_cast<float>(m_width);
    vp.Height = static_cast<float>(m_height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    return vp;
}

void RenderTextureTarget::SetClearColor(float r, float g, float b, float a)
{
    m_clearColor[0] = r;
    m_clearColor[1] = g;
    m_clearColor[2] = b;
    m_clearColor[3] = a;
}