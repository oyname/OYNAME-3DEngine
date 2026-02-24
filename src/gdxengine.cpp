#include "gdxengine.h"
#include "gdxdevice.h"   
#include "gdxwin.h"      
#include "core.h"
#include <fstream>

namespace Engine
{
	GDXEngine* engine = nullptr;

	int CreateEngine(HWND hwnd, HINSTANCE hInst, int bpp, int width, int height)
	{
		if (engine)
			return 0;

		int result = 0;
		engine = new GDXEngine(
			hwnd, hInst,
			(unsigned)bpp,
			(unsigned)width,
			(unsigned)height,
			&result
		);

		if (result != 0 || !engine)
		{
			delete engine;
			engine = nullptr;
			return (result != 0) ? result : -1;
		}

		return 0;
	}

	void ReleaseEngine()
	{
		delete engine;
		engine = nullptr;
	}
}


// Static-Variablen
bool GDXEngine::running = true;
double GDXEngine::deltaTime = 0.0;
double GDXEngine::accumulator = 0.0;
Timer::TimeMode timeMode = Timer::TimeMode::FIXED_TIMESTEP;  // Standard
std::chrono::high_resolution_clock::time_point GDXEngine::lastFrameTime = std::chrono::high_resolution_clock::now();
GDXEngine* GDXEngine::s_instance = nullptr;

// Pfad-Utilities jetzt in Core (core.h / core.cpp)
// GetExeDir() und ResolveAbsolutePath() entfernt.

//
GDXEngine::GDXEngine(HWND hwnd, HINSTANCE hinst, unsigned int bpp, unsigned int screenX, unsigned int screenY, int* result) :
	m_objectManager(),
	m_lightManager(),
	m_renderManager(m_objectManager, m_lightManager, m_device)
{
	m_colorDepth = bpp;
	m_screenWidth = screenX;
	m_screenHeight = screenY;
	m_hwnd = hwnd;

	m_globalAmbient = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);  // Standard Ambient
	s_instance = this;  // Singleton setzen
	m_device.Init();

	m_interface.Init(bpp);

	int bestAdapter = FindBestAdapter();
	this->SetAdapter(bestAdapter);

	// Shader-Pfade ueber Core aufloesen
	vs = Core::ResolvePath(L"..\\..\\shaders\\VertexShader.hlsl");
	ps = Core::ResolvePath(L"..\\..\\shaders\\PixelShader.hlsl");

	// Prüfe ob Dateien existieren
	std::wifstream vsFile(vs);
	std::wifstream psFile(ps);

	if (!vsFile.good()) {
		Debug::Log("gdxengine.cpp: Vertex Shader NOT FOUND at: ", vs.c_str());
	}
	else {
		Debug::Log("gdxengine.cpp: Vertex Shader found at: ", vs.c_str());
	}

	if (!psFile.good()) {
		Debug::Log("gdxengine.cpp: Pixel Shader NOT FOUND at: ", ps.c_str());
	}
	else {
		Debug::Log("gdxengine.cpp: Pixel Shader found at: ", ps.c_str());
	}

	m_bInitialized = true;
}

int GDXEngine::FindBestAdapter()
{
	int bestIndex = 0;
	D3D_FEATURE_LEVEL bestLevel = D3D_FEATURE_LEVEL_9_1;

	// Iteriere durch alle Adapters
	for (size_t i = 0; i < m_device.deviceManager.GetNumAdapters(); ++i)
	{
		D3D_FEATURE_LEVEL currentLevel =
			GXUTIL::GetFeatureLevelFromDirectXVersion(
				m_device.deviceManager.GetFeatureLevel(i));

		// Ist dieser besser als bisheriger beste?
		if (currentLevel > bestLevel)
		{
			bestLevel = currentLevel;
			bestIndex = i;
		}
	}

	Debug::Log("gdxengine.cpp: SELECTED BEST Adapter at index ", bestIndex,
		" with Feature Level: ", GXUTIL::GetFeatureLevelName(GXUTIL::GetFeatureLevelFromDirectXVersion(
			m_device.deviceManager.GetFeatureLevel(bestIndex))));

	return bestIndex;
}

GDXEngine::~GDXEngine()
{
	if (m_bInitialized)
		this->Cleanup();
}

void GDXEngine::Cleanup()
{
	if (m_bInitialized)
	{
		// Clean-up operations
	}

	m_bInitialized = false;
}

HRESULT GDXEngine::Graphic(unsigned int width, unsigned int height, bool windowed)
{
	HRESULT hr = S_OK;

	// Index of current adapter (0 = Primary)
	int index = GetAdapterIndex();

	// Adapter
	IDXGIAdapter* adapter = nullptr;

	// Current adapter with supported DirectX version
	D3D_FEATURE_LEVEL featureLevel;
	featureLevel = GXUTIL::GetFeatureLevelFromDirectXVersion(m_device.deviceManager.GetFeatureLevel(index));

	// Create adapter with current index
	hr = m_interface.GetFactory()->EnumAdapters(index, &adapter);
	if (FAILED(hr))
	{
		Debug::LogHr(__FILE__, __LINE__, hr);
		return hr;
	}

	// Create device
	hr = m_device.InitializeDirectX(adapter, &featureLevel);
	if (FAILED(hr))
	{
		Debug::LogHr(__FILE__, __LINE__, hr);
		return hr;
	}

	Memory::SafeRelease(adapter);

	// Frequency
	unsigned int numerator = m_interface.interfaceManager.GetNumerator(this->GetAdapterIndex(), this->GetOutputIndex(), width, height);
	unsigned int denominator = m_interface.interfaceManager.GetDenominator(this->GetAdapterIndex(), this->GetOutputIndex(), width, height);

	// Create the SwapChain
	hr = m_device.CreateSwapChain(m_interface.GetFactory(), GetHWND(), width, height, m_interface.interfaceManager.GetDXGI_Format(), numerator, denominator, windowed);
	if (FAILED(hr))
	{
		Debug::LogHr(__FILE__, __LINE__, hr);
		return hr;
	}

	// Create Backbuffer
	hr = m_device.CreateRenderTarget(width, height);
	if (FAILED(hr))
	{
		Debug::LogHr(__FILE__, __LINE__, hr);
		return hr;
	}

	// Initialization of the Depth-Stencil buffer and views
	hr = m_device.CreateDepthBuffer(width, height);
	if (FAILED(hr))
	{
		Debug::LogHr(__FILE__, __LINE__, hr);
		return hr;
	}

	hr = m_device.CreateShadowBuffer(2048, 2048);
	if (FAILED(hr))
	{
		Debug::LogHr(__FILE__, __LINE__, hr);
		return hr;
	}

	// 1. Initialize objects
	//
	// Object manager
	m_objectManager.Init();

	// Initialize Buffer Manager
	m_bufferManager.Init(m_device.GetDevice(), m_device.GetDeviceContext());

	// Initialize Shader Manager
	m_shaderManager.Init(m_device.GetDevice());

	// Initialize Input-Layout Managers
	m_inputLayoutManager.Init(m_device.GetDevice());

	// Create standard shader.
	GetSM().SetShader(m_objectManager.CreateShader());

	// ...and load - VERWENDE DIE DYNAMISCHEN PFADE!
	hr = GetSM().CreateShader(GetSM().GetShader(), vs.c_str(), "main", ps.c_str(), "main");
	if (FAILED(hr))
	{
		Debug::LogHr(__FILE__, __LINE__, hr);
		return hr;
	}

	// Create standard material and add to standard shader
	GetOM().AddMaterialToShader(GetSM().GetShader(), GetOM().CreateMaterial());

	// ==================== CREATE MATERIAL CONSTANT BUFFER ====================
	LPMATERIAL standardMaterial = GetSM().GetShader()->materials.front();

	hr = InitMaterialBuffer(standardMaterial);
	if (FAILED(hr))
	{
		Debug::LogHr(__FILE__, __LINE__, hr);
		return hr;
	}

	// Create layout for the vertices
	hr = GetILM().CreateInputLayoutVertex(&GetSM().GetShader()->inputlayoutVertex,	// Store the layout
		GetSM().GetShader(),														// The shader object
		GetSM().GetShader()->flagsVertex,											// Store the flag
		D3DVERTEX_POSITION | D3DVERTEX_COLOR | D3DVERTEX_NORMAL | D3DVERTEX_TEX1 | D3DVERTEX_TEX2);

	if (FAILED(hr))
	{
		Debug::LogHr(__FILE__, __LINE__, hr);
		return hr;
	}

	m_screenHeight = height;
	m_screenWidth = width;

	return hr;
}

HRESULT GDXEngine::RenderWorld()
{
	HRESULT hr = S_OK;

	auto* pContext = this->m_device.GetDeviceContext();
	if (!pContext)
	{
		Debug::Log("gdxengine.cpp: RenderWorld - Device Context is null.");
		return E_FAIL;
	}

	auto* pCamera = m_cameraManager.GetCurrentCam();
	if (!pCamera)
	{
		Debug::Log("gdxengine.cpp: RenderWorld - No valid camera found.");
		return E_FAIL;
	}

	// Clear
	ID3D11DepthStencilView* dsv = this->m_device.GetDepthStencilView();
	if (!dsv)
		return E_POINTER;

	pContext->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Wichtig: RenderManager bekommt deterministisch die Camera
	m_renderManager.SetCamera(pCamera);

	// RenderManager setzt OM/RS/Viewport pro Pass selbst
	m_renderManager.RenderScene();

	return hr;
}

void GDXEngine::UpdateWorld()
{
	auto* cam = m_cameraManager.GetCurrentCam();

	if (cam == nullptr) {
		Debug::Log("gdxengine.cpp: ERROR - UpdateWorld - No camera set");
		return;
	}

	DirectX::XMVECTOR position = cam->transform.GetPosition();
	DirectX::XMVECTOR forward = DirectX::XMVector3Normalize(cam->transform.GetLookAt());
	DirectX::XMVECTOR up = DirectX::XMVector3Normalize(cam->transform.GetUp());

	// Funktioniert - cam ist Camera*
	cam->UpdateCamera(position, forward, up);

	// Light
	// m_lightManager.Update(&m_device);
}

HRESULT GDXEngine::Cls(float r, float g, float b, float a)
{
	HRESULT hr = S_OK;

	//Clear our backbuffer to the updated color
	float color[4] = { r, g, b, a };

	m_device.GetDeviceContext()->ClearRenderTargetView(m_device.GetTargetView(), color);

	hr = m_device.GetDevice()->GetDeviceRemovedReason(); // Check for device error
	if (FAILED(hr))
	{
		Debug::LogHr(__FILE__, __LINE__, hr);
		return hr;
	}

	return hr;
}

HWND GDXEngine::GetHWND()
{
	return m_hwnd;
}

unsigned int GDXEngine::GetAdapterIndex()
{
	return m_adapterIndex;
}

unsigned int GDXEngine::GetOutputIndex()
{
	return m_monitorIndex;
}

unsigned int GDXEngine::GetWidth()
{
	return m_screenWidth;
}

unsigned int GDXEngine::GetHeight()
{
	return m_screenHeight;
}

unsigned int GDXEngine::GetColorDepth()
{
	return m_colorDepth;
}

BufferManager& GDXEngine::GetBM() {
	return m_bufferManager;
}

ObjectManager& GDXEngine::GetOM() {
	return m_objectManager;
}

ShaderManager& GDXEngine::GetSM() {
	return m_shaderManager;
}

LightManager& GDXEngine::GetLM() {
	return m_lightManager;
}

InputLayoutManager& GDXEngine::GetILM() {
	return m_inputLayoutManager;
}

TextureManager& GDXEngine::GetTM() {
	return m_texturManager;
}

CameraManager& GDXEngine::GetCam() {
	return m_cameraManager;
}

void GDXEngine::SetAdapter(unsigned int index)
{
	m_adapterIndex = index;
}

void GDXEngine::SetOutput(unsigned int index)
{
	m_monitorIndex = index;
}

void GDXEngine::SetCamera(LPENTITY entity)
{
	if (entity == nullptr) {
		Debug::Log("gdxengine.cpp: ERROR - SetCamera - entity is nullptr");
		return;
	}

	// Type-Check mit dynamic_cast
	Camera* camera = dynamic_cast<Camera*>(entity);
	if (camera == nullptr) {
		Debug::Log("gdxengine.cpp: ERROR - SetCamera - Entity is not a Camera!");
		return;
	}

	GetCam().SetCamera(camera);         // Camera* an CameraManager
	m_renderManager.SetCamera(entity);  // Entity* an RenderManager (OK)
}

void GDXEngine::SetDirectionalLight(LPENTITY entity)
{
	if (entity == nullptr) {
		Debug::Log("gdxengine.cpp: ERROR - SetDirectionalLight - entity is nullptr");
		return;
	}

	// Type-Check mit dynamic_cast
	Light* light = dynamic_cast<Light*>(entity);
	if (light == nullptr) {
		Debug::Log("gdxengine.cpp: ERROR - SetDirectionalLight - Entity is not a Light!");
		return;
	}

	m_renderManager.SetDirectionalLight(entity);
	Debug::Log("gdxengine.cpp: SetDirectionalLight - Directional Light set for Shadow Mapping");
}

void GDXEngine::SetVSyncInterval(int interval) noexcept
{
	m_vsyncInterval = (interval != 0) ? 1 : 0;
}

int GDXEngine::GetVSyncInterval() const noexcept
{
	return m_vsyncInterval;
}

HRESULT GDXEngine::InitMaterialBuffer(Material* material)
{
	if (!material) return E_INVALIDARG;
	if (material->materialBuffer) return S_OK; // schon vorhanden

	return GetBM().CreateBuffer(
		&material->properties,
		sizeof(Material::MaterialData),
		1,
		D3D11_BIND_CONSTANT_BUFFER,
		&material->materialBuffer
	);
}

RenderManager& GDXEngine::GetRM() {
	return m_renderManager;
}

