#pragma once

#include <windows.h>

class DialogWindow {
public:
    static void ShowCreateEventDialog(HWND hWnd);
    static void ShowSettingsDialog(HWND hWnd);
    static void ShowAlarmDialog(HWND hWnd);
};
