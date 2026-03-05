#pragma once

// NOTE:
// gdxdevice.h is included by a lot of engine/public headers.
// Keep it lean: no <d3d11.h>, no <dxgi.h>, no gdxutil.h.

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>      // HWND, HRESULT, UINT
#include <vector>

#include <d3dcommon.h>    // D3D_FEATURE_LEVEL
#include <dxgiformat.h>   // DXGI_FORMAT

#include "gxformat.h"    // GXFORMAT (small, no DX headers)

// Forward declarations for COM interfaces (avoid heavy DX includes here)
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Texture2D;
struct ID3D11DepthStencilState;
struct ID3D11DepthStencilView;
struct ID3D11RenderTargetView;
struct ID3D11RasterizerState;
struct ID3D11ShaderResourceView;
struct ID3D11SamplerState;
struct ID3D11Buffer;
struct ID3D11VertexShader;
struct ID3D11PixelShader;

struct IDXGISwapChain;
struct IDXGIFactory;
struct IDXGIAdapter;

// Forward declaration: DX11 backend (kept out of the public header to avoid DX includes)
class Dx11RenderBackend;


struct GXDEVICE
{
	D3D_FEATURE_LEVEL featureLevel;
	unsigned int directxVersion;
	GXFORMAT supportedFormat;
};

struct DEVICEMANAGER
{
private:
	std::vector<GXDEVICE> devices;

public:
	DEVICEMANAGER() {}
	~DEVICEMANAGER() { CleanupResources(); }

	void CleanupResources()
	{
		devices.clear();
	}

	bool IsFormatSupported(unsigned int adapterIndex, GXFORMAT format)
	{
		if (adapterIndex >= devices.size())
			return false;

		return (devices[adapterIndex].supportedFormat & format) == format;
	}

	std::vector<GXDEVICE>& GetDevices()
	{
		return devices;
	}

	GXFORMAT GetSupportedFormats(unsigned int adapterIndex) const
	{
		if (adapterIndex >= devices.size())
			return GXFORMAT(0);

		return devices[adapterIndex].supportedFormat;
	}

	int GetFeatureLevel(unsigned int adapterIndex) const
	{
		if (adapterIndex >= devices.size())
			return 0;

		return devices[adapterIndex].directxVersion;
	}

	size_t GetNumAdapters() const
	{
		return devices.size();
	}
};

class GDXDevice
{
private:
	// Initialization state
	bool m_bInitialized;

	// True once a D3D device + immediate context exist.
	// (SwapChain/backbuffer may still be null until a window is created.)
	bool m_deviceReady;

	// Device and context
	ID3D11Device* m_pd3dDevice;
	ID3D11DeviceContext* m_pContext;

	// Swap chain and render target
	IDXGISwapChain* m_pSwapChain;
	ID3D11Texture2D* m_pBackBuffer;

	// Depth buffer
	ID3D11Texture2D* m_depthStencilBuffer;
	ID3D11DepthStencilState* m_depthStencilState;
	ID3D11DepthStencilView* m_depthStencilView;
	ID3D11RenderTargetView* m_pRenderTargetView;
	ID3D11RasterizerState* m_pRasterizerState;

	// Step 3: Shadow mapping resources are owned by the DX11 backend.
	// GDXDevice keeps API-stable getters that delegate to the backend.
	Dx11RenderBackend* m_dx11Backend = nullptr; // non-owning

	// Private initialization methods
	HRESULT EnumerateSystemDevices();

	// Depth buffer creation
	HRESULT CreateDepthTexture(unsigned int width, unsigned int height);
	HRESULT CreateDepthStencilView();
	HRESULT CreateRasterizerState();
	HRESULT CreateDepthStencilState();

	// Shadow map creation is delegated to the backend (see CreateShadowBuffer wrapper).

	// Window management
	void ResizeWindow(HWND hwnd, unsigned int x, unsigned int y, bool windowed);

	friend class GDXEngine;
	friend class Dx11RenderBackend;

	// Called by Dx11RenderBackend to register itself for delegation.
	void AttachDx11Backend(Dx11RenderBackend* backend) noexcept { m_dx11Backend = backend; }

public:
	DEVICEMANAGER deviceManager;  // ← Bleibt hier

public:
	GDXDevice();
	~GDXDevice();

	// Initialization
	HRESULT Init();
	void Release();

	// Device initialization
	HRESULT InitializeDirectX(IDXGIAdapter* adapter, D3D_FEATURE_LEVEL* featureLevel);

	// Swap chain and render targets
	HRESULT CreateSwapChain(
		IDXGIFactory* pDXGIFactory,
		HWND hWnd,
		unsigned int width,
		unsigned int height,
		DXGI_FORMAT format,
		unsigned int numerator,
		unsigned int denominator,
		bool windowed);

	HRESULT CreateRenderTarget(unsigned int width, unsigned int height);
	HRESULT CreateDepthBuffer(unsigned int width, unsigned int height);
	HRESULT CreateShadowBuffer(unsigned int width, unsigned int height);

	// Shader setting
	void SetVertexShader(ID3D11VertexShader* vertexShader);
	void SetPixelShader(ID3D11PixelShader* pixelShader);

	// Presentation
	HRESULT Flip(int syncInterval);

	// Getters
	ID3D11Device* GetDevice() const
	{
		return m_pd3dDevice;
	}

	ID3D11RasterizerState* GetRasterizerState() const
	{
		return m_pRasterizerState;
	}

	ID3D11DeviceContext* GetDeviceContext() const
	{
		return m_pContext;
	}

	IDXGISwapChain* GetSwapChain() const
	{
		return m_pSwapChain;
	}

	ID3D11RenderTargetView* GetRenderTargetView() const
	{
		return m_pRenderTargetView;
	}

	// Rückwärtskompatibilität
	ID3D11RenderTargetView* GetTargetView() const
	{
		return m_pRenderTargetView;
	}

	ID3D11Texture2D* GetBackBuffer() const
	{
		return m_pBackBuffer;
	}

	ID3D11DepthStencilView* GetDepthStencilView() const
	{
		return m_depthStencilView;
	}

	ID3D11DepthStencilView* GetShadowMapDepthView() const
	;

	ID3D11ShaderResourceView* GetShadowMapSRV() const
	;

	ID3D11SamplerState* GetComparisonSampler() const
	;

	ID3D11RasterizerState* GetShadowRasterState() const
	;

	ID3D11Buffer* GetShadowMatrixBuffer() const
	;

	// Shadow map dimensions (queried from the texture).
	// Returns (0,0) if the shadow map is not created.
	void GetShadowMapSize(UINT& outWidth, UINT& outHeight) const;

	bool IsInitialized() const
	{
		return m_bInitialized;
	}

	bool IsDeviceReady() const noexcept
	{
		return m_deviceReady;
	}
};
