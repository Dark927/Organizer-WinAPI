#include "UIHelpers.h"
#include <tchar.h>

bool UIHelpers::DrawThemedButton(LPDRAWITEMSTRUCT dis, int theme, int cornerRadius) 
{
	if (dis->CtlType != ODT_BUTTON) 
	{
		return false;
	}

    AppStyles::ThemeColors colors = AppStyles::GetThemeColors(theme);

	// Get button text
	TCHAR buttonText[256];
	GetWindowText(dis->hwndItem, buttonText, 256);

	// Fill background with window color
	HBRUSH hBgBrush = CreateSolidBrush(colors.windowBg);
	FillRect(dis->hDC, &dis->rcItem, hBgBrush);
	DeleteObject(hBgBrush);

	// Draw rounded rectangle
	HBRUSH hBrush = CreateSolidBrush(colors.buttonBg);
	HPEN hPen = CreatePen(PS_SOLID, 1, colors.buttonEdge);

	HGDIOBJ hOldBrush = SelectObject(dis->hDC, hBrush);
	HGDIOBJ hOldPen = SelectObject(dis->hDC, hPen);

	RoundRect(dis->hDC, dis->rcItem.left, dis->rcItem.top,
		dis->rcItem.right, dis->rcItem.bottom,
		cornerRadius, cornerRadius);

	// Draw button text
	SetBkMode(dis->hDC, TRANSPARENT);
	SetTextColor(dis->hDC, colors.buttonText);

	RECT rcText = dis->rcItem;
	DrawText(dis->hDC, buttonText, -1, &rcText,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	// Cleanup
	SelectObject(dis->hDC, hOldBrush);
	SelectObject(dis->hDC, hOldPen);
	DeleteObject(hBrush);
	DeleteObject(hPen);

	return true;
}

void UIHelpers::InitOwnerDrawButton(HWND hDlg, int buttonId, HFONT hFont) 
{
	HWND hButton = GetDlgItem(hDlg, buttonId);
	if (hButton) 
	{
		SendMessage(hButton, WM_SETFONT, (WPARAM)hFont, TRUE);
		SetWindowLongPtr(hButton, GWL_STYLE,
			GetWindowLongPtr(hButton, GWL_STYLE) | BS_OWNERDRAW);
	}
}

void UIHelpers::ApplyTheme(HWND hWnd, int theme, bool isMainWindow)
{
    AppStyles::ThemeColors colors = AppStyles::GetThemeColors(theme);

    // Handle window-specific data for main windows
    if (isMainWindow) 
    {
        AppStyles::WindowThemeData* data = (AppStyles::WindowThemeData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if (!data) 
        {
            data = (AppStyles::WindowThemeData*)malloc(sizeof(AppStyles::WindowThemeData));
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)data);
        }
        data->theme = theme;
        data->bgColor = colors.windowBg;
        data->textColor = colors.titleText;
    }

    // Set window/dialog background
    HBRUSH hBgBrush = CreateSolidBrush(colors.windowBg);
    SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)hBgBrush);

    // Handle menu checkmarks for main window
    if (isMainWindow) 
    {
        HMENU hMenu = GetMenu(hWnd);
        if (hMenu) 
        {
            HMENU hViewMenu = GetSubMenu(hMenu, 0);
            HMENU hThemeMenu = GetSubMenu(hViewMenu, 0);

            // Clear all checkmarks
            for (int i = 0; i < GetMenuItemCount(hThemeMenu); i++) 
            {
                MENUITEMINFO mii = { sizeof(MENUITEMINFO) };
                mii.fMask = MIIM_STATE;
                mii.fState &= ~MFS_CHECKED;
                SetMenuItemInfo(hThemeMenu, i, TRUE, &mii);
            }

            // Set checkmark for current theme
            MENUITEMINFO mii = { sizeof(MENUITEMINFO) };
            mii.fMask = MIIM_STATE;
            mii.fState = MFS_CHECKED;

            switch (theme) 
            {
            case THEME_LIGHT: SetMenuItemInfo(hThemeMenu, 0, TRUE, &mii); break;
            case THEME_DARK: SetMenuItemInfo(hThemeMenu, 1, TRUE, &mii); break;
            case THEME_SYSTEM: SetMenuItemInfo(hThemeMenu, 2, TRUE, &mii); break;
            case THEME_CLASSIC: SetMenuItemInfo(hThemeMenu, 3, TRUE, &mii); break;
            case THEME_BLUE: SetMenuItemInfo(hThemeMenu, 5, TRUE, &mii); break;
            case THEME_GREEN: SetMenuItemInfo(hThemeMenu, 6, TRUE, &mii); break;
            case THEME_PURPLE: SetMenuItemInfo(hThemeMenu, 7, TRUE, &mii); break;
            case THEME_HIGHCONTRAST: SetMenuItemInfo(hThemeMenu, 8, TRUE, &mii); break;
            }
        }
    }

    // Apply theme to all child controls
    HWND hChild = GetWindow(hWnd, GW_CHILD);
    while (hChild) 
    {
        TCHAR className[32];
        GetClassName(hChild, className, 32);

        if (_tcscmp(className, _T("Button")) == 0) 
        {
            if (GetWindowLongPtr(hChild, GWL_STYLE) & BS_OWNERDRAW) 
            {
                InvalidateRect(hChild, NULL, TRUE);
            }
            else 
            {
                SetWindowTheme(hChild,
                    theme == THEME_CLASSIC || theme == THEME_HIGHCONTRAST ?
                    L"" : L"Explorer", NULL);
            }
        }
        hChild = GetWindow(hChild, GW_HWNDNEXT);
    }

    // Redraw the window/dialog
    RedrawWindow(hWnd, NULL, NULL,
        RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_FRAME);
}