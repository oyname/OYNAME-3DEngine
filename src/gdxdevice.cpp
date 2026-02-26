#include "gdxutil.h"
#include "gdxdevice.h"
#include "Dx11RenderBackend.h" 
  

GDXDevice::GDXDevice() : m_bInitialized(false),
    m_deviceReady(false),
    m_pd3dDevice(nullptr),
    m_pContext(nullptr),
    m_pSwapChain(nullptr),
    m_pBackBuffer(nullptr),
    m_depthStencilBuffer(nullptr),
    m_depthStencilState(nullptr),
    m_depthStencilView(nullptr),
    m_pRenderTargetView(nullptr),
    m_pRasterizerState(nullptr),
    m_dx11Backend(nullptr)
{
}

GDXDevice::~GDXDevice()
{
    Release();
}

void GDXDevice::Release()
{
    if (m_bInitialized)
    {
        if (m_pSwapChain != nullptr)
            m_pSwapChain->SetFullscreenState(FALSE, NULL);

        Memory::SafeRelease(m_pd3dDevice);
        Memory::SafeRelease(m_pContext);
        Memory::SafeRelease(m_pSwapChain);
        Memory::SafeRelease(m_pBackBuffer);
        Memory::SafeRelease(m_depthStencilBuffer);
        Memory::SafeRelease(m_depthStencilState);
        Memory::SafeRelease(m_depthStencilView);
        Memory::SafeRelease(m_pRenderTargetView);
        Memory::SafeRelease(m_pRasterizerState);

        // Shadow resources are owned by the DX11 backend.
    }

    m_bInitialized = false;
    m_deviceReady = false;
}

HRESULT GDXDevice::Init()
{
    return EnumerateSystemDevices();
}

void GDXDevice::GetShadowMapSize(UINT& outWidth, UINT& outHeight) const
{
    outWidth = 0;
    outHeight = 0;

    if (!m_dx11Backend) return;
    const UINT s = m_dx11Backend->GetShadow().GetSize();
    outWidth = s;
    outHeight = s;
}

ID3D11DepthStencilView* GDXDevice::GetShadowMapDepthView() const
{
    return (m_dx11Backend) ? m_dx11Backend->GetShadow().GetDSV() : nullptr;
}

ID3D11ShaderResourceView* GDXDevice::GetShadowMapSRV() const
{
    return (m_dx11Backend) ? m_dx11Backend->GetShadow().GetSRV() : nullptr;
}

ID3D11SamplerState* GDXDevice::GetComparisonSampler() const
{
    return (m_dx11Backend) ? m_dx11Backend->GetShadow().GetSampler() : nullptr;
}

ID3D11RasterizerState* GDXDevice::GetShadowRasterState() const
{
    return (m_dx11Backend) ? m_dx11Backend->GetShadow().GetRasterState() : nullptr;
}

ID3D11Buffer* GDXDevice::GetShadowMatrixBuffer() const
{
    return (m_dx11Backend) ? m_dx11Backend->GetShadow().GetMatrixCB() : nullptr;
}

HRESULT GDXDevice::EnumerateSystemDevices()
{
    Debug::Log("gdxdevice.cpp: EnumerateSystemDevices START...");

    ComPtr<IDXGIFactory> factory;
    HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)factory.GetAddressOf());
    if (FAILED(hr))
    {
        Debug::LogError("gdxdevice.cpp: CreateDXGIFactory failed: 0x", std::hex, hr);
        return hr;
    }

    deviceManager.GetDevices().clear();

    const D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };

    int deviceCount = 0;

    for (UINT i = 0; ; ++i)
    {
        ComPtr<IDXGIAdapter> adapter;
        hr = factory->EnumAdapters(i, adapter.GetAddressOf());
        if (hr == DXGI_ERROR_NOT_FOUND)
            break;
        if (FAILED(hr) || !adapter)
            continue;

        if (GXUTIL::isWindowsRenderer(adapter.Get()))
        {
            Debug::LogWarning("gdxdevice.cpp: Skipping Windows Basic Renderer");
            continue;
        }

        ComPtr<ID3D11Device> testDevice;
        D3D_FEATURE_LEVEL achieved = D3D_FEATURE_LEVEL_9_1;

        hr = D3D11CreateDevice(
            adapter.Get(),
            D3D_DRIVER_TYPE_UNKNOWN,
            nullptr,
            0,
            featureLevels,
            (UINT)ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION,
            testDevice.GetAddressOf(),
            &achieved,
            nullptr
        );

        if (SUCCEEDED(hr) && testDevice)
        {
            GXDEVICE gxDevice = {};
            gxDevice.featureLevel = achieved;
            gxDevice.supportedFormat = GXUTIL::GetSupportedFormats(achieved);
            gxDevice.directxVersion = GXUTIL::GetFeatureLevel(achieved);

            deviceManager.GetDevices().push_back(gxDevice);

            Debug::Log("gdxdevice.cpp: Device ", i, " added - FeatureLevel: ",
                GXUTIL::GetFeatureLevelName(achieved));

            ++deviceCount;
        }
        else
        {
            Debug::LogWarning("gdxdevice.cpp: Adapter ", i, " - D3D11CreateDevice failed (hr=0x",
                std::hex, hr, std::dec, ")");
        }
    }

    if (deviceCount == 0)
    {
        Debug::LogError("gdxdevice.cpp: CRITICAL: No suitable Direct3D devices found!");
        m_bInitialized = false;
        return E_FAIL;
    }

    Debug::Log("gdxdevice.cpp: EnumerateSystemDevices completed - ", deviceCount, " device(s) found");
    Debug::Log("gdxdevice.cpp: ...EnumerateSystemDevices END");

    m_bInitialized = true;
    return S_OK;
}

HRESULT GDXDevice::InitializeDirectX(IDXGIAdapter* adapter, D3D_FEATURE_LEVEL* featureLevel)
{
    if (!adapter || !featureLevel)
        return E_INVALIDARG;

    // Keep state consistent if re-initialized.
    m_deviceReady = false;

    HRESULT hr = S_OK;

    // Create the Direct3D device with single feature level
    hr = D3D11CreateDevice(
        adapter,
        D3D_DRIVER_TYPE_UNKNOWN,
        nullptr,
        0,
        featureLevel,   
        1,              
        D3D11_SDK_VERSION,
        &m_pd3dDevice,
        nullptr,
        &m_pContext);

    if (FAILED(hr))
    {
        Debug::LogHr(__FILE__, __LINE__, hr);
        return hr;
    }

    m_deviceReady = (m_pd3dDevice != nullptr) && (m_pContext != nullptr);
    return hr;
}

HRESULT GDXDevice::CreateSwapChain(IDXGIFactory* pDXGIFactory, HWND hWnd,
    unsigned int width, unsigned int height, DXGI_FORMAT format,
    unsigned int numerator, unsigned int denominator,
    bool windowed)
{
    if (!pDXGIFactory || !hWnd || !m_pd3dDevice)
        return E_INVALIDARG;

    HRESULT hr = S_OK;

    // Adjust window size
    ResizeWindow(hWnd, width, height, windowed);

    // Swap Chain Description
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = width;
    swapChainDesc.BufferDesc.Height = height;
    swapChainDesc.BufferDesc.Format = (format != DXGI_FORMAT_UNKNOWN) ? format : DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = (numerator != 0) ? numerator : 1;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = (denominator != 0) ? denominator : 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hWnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = windowed;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    // Create Swap Chain
    hr = pDXGIFactory->CreateSwapChain(m_pd3dDevice, &swapChainDesc, &m_pSwapChain);
    if (FAILED(hr))
    {
        Debug::LogHr(__FILE__, __LINE__, hr);
        return hr;
    }

    // Don't release factory here - caller manages it

    return hr;
}

HRESULT GDXDevice::CreateRenderTarget(unsigned int width, unsigned int height)
{
    // Width and height are determined by the swap chain, 
    // these parameters are kept for backward compatibility but not used
    if (!m_pSwapChain || !m_pd3dDevice)
        return E_INVALIDARG;

    HRESULT hr = S_OK;

    // Get back-buffer from Swap Chain
    hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&m_pBackBuffer);
    if (FAILED(hr))
    {
        Debug::LogHr(__FILE__, __LINE__, hr);
        return hr;
    }

    // Create render target view
    hr = m_pd3dDevice->CreateRenderTargetView(m_pBackBuffer, NULL, &m_pRenderTargetView);
    if (FAILED(hr))
    {
        Debug::LogHr(__FILE__, __LINE__, hr);
        return hr;
    }

    Memory::SafeRelease(m_pBackBuffer);

    return hr;
}

HRESULT GDXDevice::Flip(int syncInterval)
{
    if (!m_pSwapChain)
        return E_INVALIDARG;

    return m_pSwapChain->Present(syncInterval, 0);
}

void GDXDevice::SetVertexShader(ID3D11VertexShader* vs)
{
    if (m_pContext)
        m_pContext->VSSetShader(vs, nullptr, 0);
}

void GDXDevice::SetPixelShader(ID3D11PixelShader* ps)
{
    if (m_pContext)
        m_pContext->PSSetShader(ps, nullptr, 0);
}

void GDXDevice::ResizeWindow(HWND hwnd, unsigned int x, unsigned int y, bool windowed)
{
    if (!hwnd || x == 0 || y == 0)
        return;

    RECT rc;
    SetRect(&rc, 0, 0, x, y);

    if (windowed)
    {
        AdjustWindowRect(&rc, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_OVERLAPPED, false);
        SetWindowLong(hwnd, GWL_STYLE, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_OVERLAPPED);
    }
    else
    {
        SetWindowLong(hwnd, GWL_STYLE, WS_POPUP);
    }

    MoveWindow(hwnd,
        GetSystemMetrics(SM_CXSCREEN) / 2 - x / 2,
        GetSystemMetrics(SM_CYSCREEN) / 2 - y / 2,
        (rc.right - rc.left), (rc.bottom - rc.top), true);

    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);
}

HRESULT GDXDevice::CreateDepthBuffer(unsigned int width, unsigned int height)
{
    if (width == 0 || height == 0 || !m_pd3dDevice)
        return E_INVALIDARG;

    HRESULT hr = S_OK;

    // Create depth texture
    hr = CreateDepthTexture(width, height);
    if (FAILED(hr))
        return hr;

    // Create depth stencil view
    hr = CreateDepthStencilView();
    if (FAILED(hr))
        goto cleanup;

    // Create rasterizer state
    hr = CreateRasterizerState();
    if (FAILED(hr))
        goto cleanup;

    // Create depth stencil state
    hr = CreateDepthStencilState();
    if (FAILED(hr))
        goto cleanup;

    return hr;

cleanup:
    Memory::SafeRelease(m_depthStencilBuffer);
    Memory::SafeRelease(m_depthStencilView);
    Memory::SafeRelease(m_pRasterizerState);
    return hr;
}

HRESULT GDXDevice::CreateShadowBuffer(unsigned int width, unsigned int height)
{
    if (width == 0 || height == 0)
        return E_INVALIDARG;

    // Step 3: shadow resources are created/owned by the DX11 backend.
    if (!m_dx11Backend)
    {
        Debug::LogError("gdxdevice.cpp: CreateShadowBuffer called but no DX11 backend is attached.");
        return E_FAIL;
    }

    const bool ok = m_dx11Backend->EnsureShadowCreated(*this, width, height);
    return ok ? S_OK : E_FAIL;
}

HRESULT GDXDevice::CreateDepthTexture(unsigned int width, unsigned int height)
{
    if (!m_pd3dDevice)
        return E_INVALIDARG;

    D3D11_TEXTURE2D_DESC depthBufferDesc;
    ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));
    depthBufferDesc.Width = width;
    depthBufferDesc.Height = height;
    depthBufferDesc.MipLevels = 1;
    depthBufferDesc.ArraySize = 1;
    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.SampleDesc.Count = 1;
    depthBufferDesc.SampleDesc.Quality = 0;
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthBufferDesc.CPUAccessFlags = 0;
    depthBufferDesc.MiscFlags = 0;

    HRESULT hr = m_pd3dDevice->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
    if (FAILED(hr))
    {
        Debug::LogHr(__FILE__, __LINE__, hr);
        return hr;
    }

    return hr;
}

HRESULT GDXDevice::CreateDepthStencilView()
{
    if (!m_pd3dDevice || !m_depthStencilBuffer)
        return E_INVALIDARG;

    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    HRESULT hr = m_pd3dDevice->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
    if (FAILED(hr))
    {
        Debug::LogHr(__FILE__, __LINE__, hr);
        return hr;
    }

    return hr;
}

HRESULT GDXDevice::CreateRasterizerState()
{
    if (!m_pd3dDevice)
        return E_INVALIDARG;

    D3D11_RASTERIZER_DESC rasterizerDesc;
    ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_BACK;
    rasterizerDesc.FrontCounterClockwise = FALSE;
    rasterizerDesc.DepthClipEnable = TRUE;

    HRESULT hr = m_pd3dDevice->CreateRasterizerState(&rasterizerDesc, &m_pRasterizerState);
    if (FAILED(hr))
    {
        Debug::LogHr(__FILE__, __LINE__, hr);
        return hr;
    }

    return hr;
}

HRESULT GDXDevice::CreateDepthStencilState()
{
    if (!m_pd3dDevice)
        return E_INVALIDARG;

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    depthStencilDesc.StencilEnable = false;

    HRESULT hr = m_pd3dDevice->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
    if (FAILED(hr))
    {
        Debug::LogHr(__FILE__, __LINE__, hr);
        return hr;
    }

    return hr;
}

// Step 3:
// Shadow resource creation moved to Dx11ShadowMap (owned by Dx11RenderBackend).