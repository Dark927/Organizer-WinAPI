#pragma once
#include <windows.h>
#include "AppStyles.h"

// Add this global variable declaration
extern int currentTheme;

// Update dialog functions to accept theme parameter
void ShowAboutDialog(HWND parentWnd, HINSTANCE hInst, int theme);
void ShowEventsDialog(HWND parentWnd, HINSTANCE hInst, int theme);