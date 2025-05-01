#pragma once

#include <windows.h>
#include "AppStyles.h"

class UIHelpers 
{
public:
    // Draws a themed button with rounded corners
    static bool DrawThemedButton(LPDRAWITEMSTRUCT dis, int theme, int cornerRadius = 8);

    // Initializes a button for owner-draw
    static void InitOwnerDrawButton(HWND hDlg, int buttonId, HFONT hFont);

    // Applies theme to a dialog and its controls
    static void ApplyTheme(HWND hWnd, int theme, bool isMainWindow = false);
};