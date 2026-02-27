#pragma once

#include <Windows.h>
#include "core.h"
#include "gdxwin.h"
#include "gdxutil.h"
#include "gdxinterface.h"
#include "gdxdevice.h"
#include "InputLayoutManager.h"
#include "ObjectManager.h"
#include "BufferManager.h"
#include "ShaderManager.h"
#include "RenderManager.h"
#include "LightManager.h"
#include "TextureManager.h"
#include "TexturePool.h"
#include "CameraManager.h"
#include "Transform.h"
#include "Timer.h"
#include <thread>

#define VERTEX_SHADER_FILE L"shaders/VertexShader.hlsl" 
#define PIXEL_SHADER_FILE L"shaders/PixelShader.hlsl"
// Forward declaration
class GDXEngine;

namespace Engine
{
	extern GDXEngine* engine;

	int  CreateEngine(HWND hwnd, HINSTANCE hInst, int bpp, int width, int height);
	void ReleaseEngine();
}


class GDXEngine {
private:
	bool m_bInitialized;
	HWND m_hwnd;						// Window handle
	int m_colorDepth;					// Color depth (e.g., 32-bit colors)
	int m_screenWidth;					// Screen width
	int m_screenHeight;					// Screen height	
	int m_adapterIndex;					// Current adapter index
	int m_monitorIndex;					// Index of the monitor
	static GDXEngine* s_instance;		// Singleton-Pointer

	DirectX::XMFLOAT4 m_globalAmbient;  // Globale Ambient-Farbe für die Szene

	std::wstring vs;
	std::wstring ps;

	// Manager classes
	ObjectManager       m_objectManager;
	RenderManager		m_renderManager;
	ShaderManager		m_shaderManager;
	InputLayoutManager	m_inputLayoutManager;
	BufferManager		m_bufferManager;
	LightManager		m_lightManager;
	TextureManager		m_texturManager;
	TexturePool			m_texturePool;
	CameraManager			m_cameraManager;

	int m_vsyncInterval = 1; // 1=ON, 0=OFF

public:
	GDXDevice m_device;		    // Device Manager, not to be confused with DirectXDevice
	GDXInterface m_interface;	// Interface Manager

	GDXEngine(HWND hwnd, HINSTANCE hinst, unsigned int bpp, unsigned int screenX, unsigned int screenY, int* result);
	~GDXEngine();

	static GDXEngine* GetInstance() { return s_instance; }

	HRESULT RenderWorld();
	void Cleanup();
	void UpdateWorld();

	// 
	HRESULT Graphic(unsigned int width, unsigned int height, bool windowed);
	HRESULT Cls(float r, float g, float b, float a);
	int FindBestAdapter();

	// Getter-Funktionen fÃ¼r private Variablen
	HWND GetHWND();
	unsigned int GetAdapterIndex();
	unsigned int GetOutputIndex();
	unsigned int GetWidth();
	unsigned int GetHeight();
	unsigned int GetColorDepth();

	BufferManager& GetBM();			// BufferManager
	ObjectManager& GetOM();			// ObjectManager
	ShaderManager& GetSM();			// ShaderManager
	LightManager& GetLM();			// LightManager
	InputLayoutManager& GetILM();	// InputManager
	TextureManager& GetTM();		// TextureManager
	TexturePool& GetTP();			// TexturePool
	CameraManager& GetCam();		// KameraManager
	RenderManager& GetRM();			// RenderManager

	// Setter-Funktionen fÃ¼r private Variablen
	void SetAdapter(unsigned int index);
	void SetOutput(unsigned int index);
	void SetDirectionalLight(LPENTITY entity);
	void SetGlobalAmbient(const DirectX::XMFLOAT4& ambient) { m_globalAmbient = ambient; }
	void SetCamera(LPENTITY mesh);
	void SetVSyncInterval(int interval) noexcept;

	// Legt den Constant Buffer fuer ein Material an.
	// Muss nach CreateMaterial() fuer jedes Material aufgerufen werden.
	HRESULT InitMaterialBuffer(Material* material);


	DirectX::XMFLOAT4 GetGlobalAmbient() const { return m_globalAmbient; }
	int GetVSyncInterval() const noexcept;


private:
	static bool running;
	static double deltaTime;
	static double accumulator;
	static std::chrono::high_resolution_clock::time_point lastFrameTime;

	static constexpr double FIXED_TIMESTEP = 1.0 / 60.0;
	static constexpr double TARGET_FPS = 60.0;
};
