#include "main.h"
#include "core.h"
#include "gdxwin.h"

#include <thread>
#include <atomic>

INT APIENTRY WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int)
{
    // 1) Core initialisieren (COM, Fenster, Timer, Pfade)
    Core::Desc desc;
    desc.vsync       = true;
    desc.debug       = true;
    desc.windowed    = true;
    desc.windowTitle = L"OYNAME Engine";
    desc.className   = L"gidx";
    desc.appIcon     = 101;
    // desc.width / desc.height = 0 -> Desktop-Aufloesung

    HWND hwnd = Core::Init(hInst, WindowProc, desc);
    if (!hwnd)
    {
        MessageBox(nullptr, L"Fatal: Core::Init failed.", L"ERROR", MB_OK);
        return -1;
    }

    // 2) DirectX-Engine erstellen
    int rc = Core::CreateEngine();
    if (rc != 0)
    {
        MessageBox(nullptr, L"Fatal: Core::CreateEngine failed.", L"ERROR", MB_OK);
        Core::Shutdown();
        return 0;
    }

    // 3) Game-Loop auf separatem Thread starten
    std::atomic_bool threadFinished{ false };
    std::thread mainThread([&]()
        {
            main(reinterpret_cast<LPVOID>(hwnd));
            threadFinished = true;
        });

    // 4) Message-Loop (Hauptthread)
    MSG msg{};
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // 5) Game-Thread beenden
    if (mainThread.joinable())
        mainThread.join();

    // 6) Alles herunterfahren (Engine -> Timer -> COM)
    Core::Shutdown();

    return (int)msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
        {
            PostMessage(hWnd, WM_CLOSE, 0, 0);
            return 0;
        }
        break;

    case WM_CLOSE:
        Windows::MainLoop(false);
        DestroyWindow(hWnd);
        return 0;

    case WM_DESTROY:
        Windows::MainLoop(false);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}
