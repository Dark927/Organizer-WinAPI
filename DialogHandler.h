#pragma once
#include <windows.h>
#include "AppStyles.h"

extern int currentTheme;

void ShowAboutDialog(HWND parentWnd, HINSTANCE hInst, int theme);
void ShowEventsDialog(HWND parentWnd, HINSTANCE hInst, int theme);