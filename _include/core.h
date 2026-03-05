#pragma once

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <cstdint>

// ============================================================
// Core – Bootstrap- und Lifecycle-Schicht
//
// Verantwortlich fuer:
//   1. Application Lifecycle  (COM, Window, Thread-Signal)
//   2. Globale Konfiguration  (einzige Quelle fuer vsync, debug, windowed, ...)
//   3. Pfad-Management        (ExeDir, ResolvePath)
//   4. Frame-Timing           (DeltaTime, FPS, FrameCount)
//   5. Debug/Logging-Init     (Console, Log-Level)
//   6. Shutdown-Koordination  (Manager -> Device -> COM)
//
// Core kennt KEIN ID3D11Device und kein IDXGISwapChain.
// ============================================================

namespace Core
{
    // ----------------------------------------------------------
    // Globale Konfiguration 
    // ----------------------------------------------------------
    struct Desc
    {
        // Fenster
        int             width       = 0;    // 0 = Desktop-Breite
        int             height      = 0;    // 0 = Desktop-Hoehe
        bool            windowed    = true;

        // Rendering
        bool            vsync       = true;

        // Debug
        bool            debug       = true;

        // Fenster-Titel und Klassenname
        const wchar_t*  windowTitle = L"giDX\u00b3 Engine";
        const wchar_t*  className   = L"gidx";
        WORD            appIcon     = 101;
    };

    // ----------------------------------------------------------
    // Lifecycle
    // ----------------------------------------------------------

    // Initialisiert COM, erstellt Fenster, startet Timer, loest Pfade auf.
    // Gibt das HWND zurueck, oder nullptr bei Fehler.
    HWND Init(HINSTANCE hInst, WNDPROC wndProc, const Desc& desc);

    // Erstellt die DirectX-Engine (delegiert an Engine::CreateEngine).
    // Muss nach Init() aufgerufen werden.
    int  CreateEngine();

    // Frame-Beginn: Timer::Tick, Frame-Counter.
    void BeginFrame();

    // Frame-Ende: Platzhalter fuer Frame-Limiter, Stats.
    void EndFrame();

    // Faehrt alles herunter:  Engine -> Timer -> COM.
    void Shutdown();

    // Ist die Engine aktiv?
    bool IsRunning();

    // Konfiguration (nur lesen)
    const Desc& GetDesc();
    HWND        GetHWND();
    HINSTANCE   GetHInstance();


    // Pfad-Utilities (frueher in gdxengine.cpp)

    // Verzeichnis der ausfuehrbaren Datei inkl. trailing backslash.
    const std::wstring& GetExeDir();

    // Loest relativen Pfad relativ zum ExeDir auf.
    std::wstring ResolvePath(const std::wstring& relativePath);

    // Frame-Statistiken
    double   GetDeltaTime();
    double   GetFPS();
    uint64_t GetFrameCount();
}
