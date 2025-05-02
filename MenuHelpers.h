#pragma once
#include <windows.h>

class MenuHelpers
{
public:
    static void InitializeFileMenu(HMENU hMenu);
    static bool HandleMenuCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
};