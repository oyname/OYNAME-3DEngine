#include "gdxengine.h"
#include "gdxdevice.h"   
#include "gdxwin.h"      
#include "core.h"
#include "Dx11MaterialGpuData.h"
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

GDXEngine::GDXEngine(HWND hwnd, HINSTANCE hinst, unsigned int bpp, unsigned int screenX, unsigned int screenY, int* result) :
	m_scene(),
	m_assetManager(),
	m_objectManager(m_scene, m_assetManager),
	m_renderManager(m_scene, m_assetManager, m_device)
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

	// Resolve shader paths via Core
	vs = Core::ResolvePath(VERTEX_SHADER_FILE);
	ps = Core::ResolvePath(PIXEL_SHADER_FILE);

	// Check if shader files exist
	std::wifstream vsFile(vs);
	std::wifstream psFile(ps);

	if (!vsFile.good()) {
		DBLOG("gdxengine.cpp: Vertex Shader NOT FOUND at: ", vs.c_str());
	}
	else {
		DBLOG("gdxengine.cpp: Vertex Shader found at: ", vs.c_str());
	}

	if (!psFile.good()) {
		DBLOG("gdxengine.cpp: Pixel Shader NOT FOUND at: ", ps.c_str());
	}
	else {
		DBLOG("gdxengine.cpp: Pixel Shader found at: ", ps.c_str());
	}

	m_bInitialized = true;
}

int GDXEngine::FindBestAdapter()
{
	int bestIndex = 0;
	D3D_FEATURE_LEVEL bestLevel = D3D_FEATURE_LEVEL_9_1;

	for (size_t i = 0; i < m_device.deviceManager.GetNumAdapters(); ++i)
	{
		D3D_FEATURE_LEVEL currentLevel =
			GXUTIL::GetFeatureLevelFromDirectXVersion(
				m_device.deviceManager.GetFeatureLevel(i));

		if (currentLevel > bestLevel)
		{
			bestLevel = currentLevel;
			bestIndex = i;
		}
	}

	DBLOG("gdxengine.cpp: SELECTED BEST Adapter at index ", bestIndex,
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
		DBLOG_HR(hr);
		return hr;
	}

	// Create device
	hr = m_device.InitializeDirectX(adapter, &featureLevel);
	if (FAILED(hr))
	{
		DBLOG_HR(hr);
		return hr;
	}
	m_renderManager.EnsureBackend();

	Memory::SafeRelease(adapter);

	// Frequency
	unsigned int numerator = m_interface.interfaceManager.GetNumerator(this->GetAdapterIndex(), this->GetOutputIndex(), width, height);
	unsigned int denominator = m_interface.interfaceManager.GetDenominator(this->GetAdapterIndex(), this->GetOutputIndex(), width, height);

	// Create the SwapChain
	hr = m_device.CreateSwapChain(m_interface.GetFactory(), GetHWND(), width, height, m_interface.interfaceManager.GetDXGI_Format(), numerator, denominator, windowed);
	if (FAILED(hr))
	{
		DBLOG_HR(hr);
		return hr;
	}

	// Create Backbuffer
	hr = m_device.CreateRenderTarget(width, height);
	if (FAILED(hr))
	{
		DBLOG_HR(hr);
		return hr;
	}

	// Initialization of the Depth-Stencil buffer and views
	hr = m_device.CreateDepthBuffer(width, height);
	if (FAILED(hr))
	{
		DBLOG_HR(hr);
		return hr;
	}

	hr = m_device.CreateShadowBuffer(2048, 2048);
	if (FAILED(hr))
	{
		DBLOG_HR(hr);
		return hr;
	}

	// 1. Initialize objects
	//
	// Scene / Asset system
	m_scene.Init();
	DBLOG("gdxengine.cpp: Graphic - Scene initialized");
	m_assetManager.Init();
	DBLOG("gdxengine.cpp: Graphic - AssetManager initialized");
	// ObjectManager::Init() is intentionally a no-op. It exists only as a compatibility stub.

	m_bufferManager.Init(m_device.GetDevice(), m_device.GetDeviceContext());
	DBLOG("gdxengine.cpp: Graphic - BufferManager initialized");

	m_shaderManager.Init(m_device.GetDevice());
	DBLOG("gdxengine.cpp: Graphic - ShaderManager initialized");

	m_inputLayoutManager.Init(m_device.GetDevice());
	DBLOG("gdxengine.cpp: Graphic - InputLayoutManager initialized");

	{
		Shader* stdShader = m_assetManager.CreateShader();
		DBLOG("gdxengine.cpp: Graphic - Standard Shader object created (id=", stdShader ? stdShader->id : 0u, ")");
		GetSM().SetShader(stdShader);
		DBLOG("gdxengine.cpp: Graphic - SetShader done");
	}

	// Load standard shader
	DBLOG("gdxengine.cpp: Graphic - compiling standard VS/PS (may take a moment)...");
	hr = GetSM().CreateShader(GetSM().GetShader(), vs.c_str(), "main", ps.c_str(), "main");
	if (FAILED(hr))
	{
		DBLOG_HR(hr);
		return hr;
	}
	DBLOG("gdxengine.cpp: Graphic - standard shader compiled OK");

	DBLOG("gdxengine.cpp: Graphic - creating standard material...");
	m_assetManager.AddMaterialToShader(GetSM().GetShader(), m_assetManager.CreateMaterial());
	DBLOG("gdxengine.cpp: Graphic - AddMaterialToShader done, materials.size=",
		(int)GetSM().GetShader()->materials.size());

	if (GetSM().GetShader()->materials.empty())
	{
		DBERROR("gdxengine.cpp: Graphic - materials vector is EMPTY – cannot get standard material!");
		return E_FAIL;
	}

	LPMATERIAL standardMaterial = GetSM().GetShader()->materials.front();

	hr = InitMaterialBuffer(standardMaterial);
	if (FAILED(hr))
	{
		DBLOG_HR(hr);
		return hr;
	}

	// Initialize TexturePool defaults (white / flat-normal / ORM)
	if (!m_texturePool.InitializeDefaults(m_device.GetDevice()))
	{
		DBERROR("gdxengine.cpp: TexturePool::InitializeDefaults fehlgeschlagen");
	}
	else
	{
		DBLOG("gdxengine.cpp: TexturePool initialisiert – Defaults bei Index 0/1/2");
		m_renderManager.SetTexturePool(&m_texturePool);
		DBLOG("gdxengine.cpp: TexturePool passed to RenderManager");
	}

	// Default material (light grey, no texture).
	// Returned by GetStandardMaterial() when CreateMesh is called without a material.
	{
		Material* defaultMat = GetAM().CreateMaterial();
		defaultMat->SetDiffuseColor(0.8f, 0.8f, 0.8f, 1.0f);
		GetAM().AddMaterialToShader(GetSM().GetShader(), defaultMat);
		hr = InitMaterialBuffer(defaultMat);
		if (FAILED(hr))
		{
			DBLOG_HR(hr);
			// Non-fatal - engine continues, GetStandardMaterial falls back to front()
		}
		else
		{
			GetAM().SetDefaultMaterial(defaultMat);
			DBLOG("gdxengine.cpp: Default material created (light grey 0.75/0.75/0.75)");
		}
	}

	hr = GetILM().CreateInputLayoutVertex(&GetSM().GetShader()->inputlayoutVertex,	// Store the layout
		GetSM().GetShader(),														// The shader object
		GetSM().GetShader()->flagsVertex,											// Store the flag
		D3DVERTEX_POSITION | D3DVERTEX_COLOR | D3DVERTEX_NORMAL | D3DVERTEX_TEX1);

	if (FAILED(hr))
	{
		DBLOG_HR(hr);
		return hr;
	}

	// Optionaler interner Skinned-Standardshader: eigener VS, gleicher PS.
	{
		std::wstring vsSkin = Core::ResolvePath(VERTEX_SKINNING_SHADER_FILE);
		Shader* skinnedShader = GetAM().CreateShader();

		if (skinnedShader == nullptr)
		{
			DBLOG("gdxengine.cpp: failed to allocate skinned standard shader");
		}
		else
		{
			const DWORD skinnedFlags =
				D3DVERTEX_POSITION | D3DVERTEX_NORMAL |
				D3DVERTEX_COLOR | D3DVERTEX_TEX1 |
				D3DVERTEX_BONE_INDICES | D3DVERTEX_BONE_WEIGHTS;

			HRESULT hrSkin = GetSM().CreateShader(skinnedShader, vsSkin.c_str(), "main", ps.c_str(), "main");
			if (FAILED(hrSkin))
			{
				DBLOG_HR(hrSkin);
			}
			else
			{
				hrSkin = GetILM().CreateInputLayoutVertex(
					&skinnedShader->inputlayoutVertex,
					skinnedShader,
					skinnedShader->flagsVertex,
					skinnedFlags);

				if (FAILED(hrSkin))
				{
					DBLOG_HR(hrSkin);
				}
				else
				{
					GetSM().SetShader(ShaderKey::StandardSkinned, skinnedShader);
					DBLOG("gdxengine.cpp: internal skinned standard shader registered");
				}
			}
		}
	}

	// Shadow-Pass VS: liest World aus b0, Light-View/Proj aus b3.
	// Gleiche Vertex-Signatur wie Standard-VS (kein Skinning).
	// PS wird nicht benoetigt (depth-only).
	{
		std::wstring vsShadow = Core::ResolvePath(VERTEX_SHADOW_SHADER_FILE);
		Shader* shadowShader = GetAM().CreateShader();

		if (!shadowShader)
		{
			DBERROR("gdxengine.cpp: Shadow-VS Shader-Objekt konnte nicht erstellt werden");
		}
		else
		{
			const DWORD shadowFlags =
				D3DVERTEX_POSITION;

			HRESULT hrShadow = GetSM().CompileShaderFromFile(
				vsShadow, "main", "vs_5_0", &shadowShader->blobVS);

			if (FAILED(hrShadow))
			{
				DBERROR("gdxengine.cpp: Shadow-VS Kompilierung fehlgeschlagen");
				DBLOG_HR(hrShadow);
			}
			else
			{
				hrShadow = m_device.GetDevice()->CreateVertexShader(
					shadowShader->blobVS->GetBufferPointer(),
					shadowShader->blobVS->GetBufferSize(),
					nullptr,
					&shadowShader->vertexShader);

				if (FAILED(hrShadow))
				{
					DBERROR("gdxengine.cpp: Shadow-VS CreateVertexShader fehlgeschlagen");
					DBLOG_HR(hrShadow);
				}
				else
				{
					hrShadow = GetILM().CreateInputLayoutVertex(
						&shadowShader->inputlayoutVertex,
						shadowShader,
						shadowShader->flagsVertex,
						shadowFlags);

					if (FAILED(hrShadow))
					{
						DBERROR("gdxengine.cpp: Shadow-VS InputLayout fehlgeschlagen");
						DBLOG_HR(hrShadow);
					}
					else
					{
						// PS bleibt nullptr: depth-only pass.
						GetSM().SetShader(ShaderKey::Shadow, shadowShader);
						m_renderManager.SetShadowShader(shadowShader);
						DBLOG("gdxengine.cpp: Shadow-VS registriert (b0=world, b3=lightViewProj)");
					}
				}
			}
		}
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
		DBLOG("gdxengine.cpp: RenderWorld - Device Context is null.");
		return E_FAIL;
	}

	auto* pCamera = m_currentCam;
	if (!pCamera)
	{
		DBLOG("gdxengine.cpp: RenderWorld - No valid camera found.");
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
	auto* cam = m_currentCam;

	if (cam == nullptr) {
		DBLOG("gdxengine.cpp: ERROR - UpdateWorld - No camera set");
		return;
	}

	DirectX::XMVECTOR position = cam->transform.GetPosition();
	DirectX::XMVECTOR forward = DirectX::XMVector3Normalize(cam->transform.GetLookAt());
	DirectX::XMVECTOR up = DirectX::XMVector3Normalize(cam->transform.GetUp());

	cam->UpdateCamera(position, forward, up);
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
		DBLOG_HR(hr);
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

Scene& GDXEngine::GetScene() {
	return m_scene;
}

AssetManager& GDXEngine::GetAM() {
	return m_assetManager;
}

ShaderManager& GDXEngine::GetSM() {
	return m_shaderManager;
}

InputLayoutManager& GDXEngine::GetILM() {
	return m_inputLayoutManager;
}

TexturePool& GDXEngine::GetTP() {
	return m_texturePool;
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
		DBLOG("gdxengine.cpp: ERROR - SetCamera - entity is nullptr");
		return;
	}

	Camera* camera = (entity->IsCamera() ? entity->AsCamera() : nullptr);
	if (camera == nullptr) {
		DBLOG("gdxengine.cpp: ERROR - SetCamera - Entity is not a Camera!");
		return;
	}

	m_currentCam = camera;
	m_renderManager.SetCamera(entity);  // Entity* an RenderManager (OK)
}

void GDXEngine::SetDirectionalLight(LPENTITY entity)
{
	if (entity == nullptr) {
		DBLOG("gdxengine.cpp: ERROR - SetDirectionalLight - entity is nullptr");
		return;
	}

	Light* light = (entity->IsLight() ? entity->AsLight() : nullptr);
	if (light == nullptr) {
		DBLOG("gdxengine.cpp: ERROR - SetDirectionalLight - Entity is not a Light!");
		return;
	}

	m_renderManager.SetDirectionalLight(entity);
	DBLOG("gdxengine.cpp: SetDirectionalLight - directional light set for shadow mapping");
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
	if (!material->gpuData) material->gpuData = new MaterialGpuData();
	if (material->gpuData->materialBuffer) return S_OK;

	constexpr UINT kMaterialCBSize = 128;

	// Zero-initialize so gTexIndex/gMisc are not random
	alignas(16) uint8_t zero[kMaterialCBSize] = {};

	return GetBM().CreateBuffer(
		zero,
		kMaterialCBSize,
		1,
		D3D11_BIND_CONSTANT_BUFFER,
		&material->gpuData->materialBuffer
	);

}

RenderManager& GDXEngine::GetRM() {
	return m_renderManager;
}

