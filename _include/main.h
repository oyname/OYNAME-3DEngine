#pragma once
#include <windows.h>

// Your window proc + your engine/game thread entry
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
int main(LPVOID hwnd);
