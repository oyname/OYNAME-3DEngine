#include "core.h"
#include "Timer.h"
#include "gdxutil.h"
#include "gdxwin.h"
#include "gdxengine.h"

#include <iostream>    // std::cout.clear(), std::cerr.clear()
#include <objbase.h>   // CoInitialize / CoUninitialize
#pragma comment(lib, "Ole32.lib")

namespace Core
{
    // Interner Zustand
    enum class State { Uninitialized, Initialized, Running, ShuttingDown, Stopped };

    static State        g_state     = State::Uninitialized;
    static Desc         g_desc{};
    static HWND         g_hwnd      = nullptr;
    static HINSTANCE    g_hInst     = nullptr;
    static uint64_t     g_frameCount = 0;
    static bool         g_comInit   = false;

    // Pfad-Cache
    static std::wstring g_exeDir;
    static bool         g_exeDirResolved = false;

    // Fenster-Objekt (Lebensdauer = Core)
    static Windows::CWindow g_window;

    // Pfad-Helpers (intern)
    static std::wstring ResolveExeDir()
    {
        wchar_t path[MAX_PATH]{};
        GetModuleFileNameW(nullptr, path, MAX_PATH);
        std::wstring p(path);
        auto pos = p.find_last_of(L"\\/");
        return (pos == std::wstring::npos) ? L"" : p.substr(0, pos + 1);
    }

    static std::wstring ResolveAbsolutePath(const std::wstring& relativePath)
    {
        wchar_t absolutePath[MAX_PATH];
        DWORD result = GetFullPathNameW(relativePath.c_str(), MAX_PATH, absolutePath, nullptr);

        if (result == 0 || result >= MAX_PATH) {
            Debug::Log("core.cpp: WARNING - Could not resolve path: ", relativePath.c_str());
            return relativePath;
        }

        return std::wstring(absolutePath);
    }

    // Lifecycle
    HWND Init(HINSTANCE hInst, WNDPROC wndProc, const Desc& desc)
    {
        if (g_state == State::Running || g_state == State::Initialized)
        {
            Debug::Log("core.cpp: Init already called - returning existing HWND");
            return g_hwnd;
        }

        g_desc  = desc;
        g_hInst = hInst;

        // 1. COM
        HRESULT hr = CoInitialize(nullptr);
        if (FAILED(hr))
        {
            Debug::LogError("core.cpp: CoInitialize failed");
            return nullptr;
        }
        g_comInit = true;

        // 2. Fenster erstellen (Console wird hier erzeugt via CreateConsole)
        //    MUSS vor jedem Debug::Log passieren, sonst setzt cout das failbit!
        g_hwnd = g_window.Init(wndProc, hInst, g_desc.className, g_desc.windowTitle, g_desc.appIcon);
        if (!g_hwnd)
        {
            CoUninitialize();
            g_comInit = false;
            return nullptr;
        }

        // cout/cerr nach freopen_s zuruecksetzen (failbit loeschen)
        std::cout.clear();
        std::cerr.clear();

        // 3. Pfade cachen (jetzt ist die Konsole da, Debug::Log funktioniert)
        g_exeDir = ResolveExeDir();
        g_exeDirResolved = true;
        Debug::Log("core.cpp: ExeDir = ", g_exeDir.c_str());

        // 4. Timer
        Timer::Init();
        Timer::Reset();

        // 5. Frame-Counter
        g_frameCount = 0;

        g_state = State::Initialized;
        Debug::Log("core.cpp: Core initialized successfully");

        return g_hwnd;
    }

    int CreateEngine()
    {
        if (g_state != State::Initialized)
        {
            Debug::LogError("core.cpp: CreateEngine called before Init() or already running");
            return -1;
        }

        // Bildschirmaufloesung bestimmen
        int w = g_desc.width;
        int h = g_desc.height;
        if (w <= 0) w = GetSystemMetrics(SM_CXSCREEN);
        if (h <= 0) h = GetSystemMetrics(SM_CYSCREEN);

        int bpp = static_cast<int>(g_window.GetColorRes());

        int rc = Engine::CreateEngine(g_hwnd, g_hInst, bpp, w, h);
        if (rc != 0)
        {
            Debug::LogError("core.cpp: Engine::CreateEngine failed with code ", rc);
            return rc;
        }

        g_state = State::Running;
        Debug::Log("core.cpp: Engine created successfully");

        return 0;
    }

    void BeginFrame()
    {
        if (g_state != State::Running) return;

        Timer::Tick();
        ++g_frameCount;
    }

    void EndFrame()
    {
        if (g_state != State::Running) return;

        // Platzhalter fuer zukuenftigen Frame-Limiter oder Stats-Sammlung
    }

    void Shutdown()
    {
        if (g_state == State::Stopped || g_state == State::Uninitialized)
            return;

        g_state = State::ShuttingDown;
        Debug::Log("core.cpp: Shutdown started");

        // 1. Engine freigeben
        Engine::ReleaseEngine();
        Debug::Log("core.cpp: Engine released");

        // 2. Timer
        Timer::Shutdown();

        // 3. COM
        if (g_comInit)
        {
            CoUninitialize();
            g_comInit = false;
        }

        g_state = State::Stopped;
        Debug::Log("core.cpp: Shutdown complete");
    }

    bool IsRunning()
    {
        return g_state == State::Running;
    }

    // Konfiguration / Getter
    const Desc& GetDesc()
    {
        return g_desc;
    }

    HWND GetHWND()
    {
        return g_hwnd;
    }

    HINSTANCE GetHInstance()
    {
        return g_hInst;
    }

    // Pfad-Utilities
    const std::wstring& GetExeDir()
    {
        if (!g_exeDirResolved)
        {
            g_exeDir = ResolveExeDir();
            g_exeDirResolved = true;
        }
        return g_exeDir;
    }

    std::wstring ResolvePath(const std::wstring& relativePath)
    {
        return ResolveAbsolutePath(GetExeDir() + relativePath);
    }

    // Frame-Statistiken
    double GetDeltaTime()
    {
        return Timer::GetDeltaTime();
    }

    double GetFPS()
    {
        return Timer::GetFPS();
    }

    uint64_t GetFrameCount()
    {
        return g_frameCount;
    }
}
