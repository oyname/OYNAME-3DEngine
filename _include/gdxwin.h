#pragma once

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>

namespace Windows
{
    void MainLoop(bool running);  // legacy setter, WindowProc nutzt das
    bool MainLoop();              // pumpt Messages / running status

    void RequestQuit();
    HWND GetHWND();
    void SetHWND(HWND hwnd);

    class CWindow
    {
    public:
        CWindow();
        ~CWindow();

        HWND Init(WNDPROC wpCallback,
            HINSTANCE hInst,
            LPCWSTR lpstrName,
            LPCWSTR lpstrTitle,
            WORD wAppIcon);

        BOOL Release(void);

        HINSTANCE GetInst(void);
        HWND      GetHwnd(void);
        unsigned int GetColorRes(void);
        void CreateConsole(void);

    private:
        bool        m_bInitialized = false;
        HINSTANCE   m_hInst = nullptr;
        HWND        m_hWnd = nullptr;
        int         m_nWidth = 0;
        int         m_nHeight = 0;
        std::wstring m_sName;
    };

    using LPWINDOWCLASS = CWindow*;
}

