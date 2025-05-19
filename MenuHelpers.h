#pragma once
#include <windows.h>

class MenuHelpers
{
public:
    static bool HandleMenuCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
};