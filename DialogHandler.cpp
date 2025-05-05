#include "DialogHandler.h"
#include <tchar.h>
#include "resource.h"
#include "UIHelpers.h"
#include <uxtheme.h>
#include "EventsDialog.h"

INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

#pragma region Dialogs

#pragma region About Dialog

void ShowAboutDialog(HWND parentWnd, HINSTANCE hInst, int theme)
{
	currentTheme = theme;
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ABOUT_DIALOG),
		parentWnd, AboutDlgProc, (LPARAM)theme);
}

INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HFONT hTitleFont;
	static HFONT hButtonFont;

	switch (message)
	{
	case WM_INITDIALOG:
	{
		currentTheme = (int)lParam;

		// Create fonts
		HDC hdc = GetDC(hDlg);
		int titleFontSize = -MulDiv(14, GetDeviceCaps(hdc, LOGPIXELSY), 72);
		int buttonFontSize = -MulDiv(9, GetDeviceCaps(hdc, LOGPIXELSY), 72);
		ReleaseDC(hDlg, hdc);

		hTitleFont = CreateFont(titleFontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
			DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
			CLEARTYPE_QUALITY, VARIABLE_PITCH, _T("Segoe UI"));

		hButtonFont = AppStyles::GetDefaultButtonFont(buttonFontSize);

		// Apply fonts to controls
		SendMessage(GetDlgItem(hDlg, IDC_STATIC_TITLE), WM_SETFONT, (WPARAM)hTitleFont, TRUE);

		UIHelpers::InitOwnerDrawButton(hDlg, ID_ABOUT_CLOSE, hButtonFont);
		UIHelpers::ApplyTheme(hDlg, currentTheme);
		return TRUE;
	}

	case WM_DRAWITEM:
		if (UIHelpers::DrawThemedButton((LPDRAWITEMSTRUCT)lParam, currentTheme))
		{
			return TRUE;
		}
		break;

	case WM_CTLCOLORDLG:
		return (LRESULT)CreateSolidBrush(AppStyles::GetThemeColors(currentTheme).windowBg);

	case WM_CTLCOLORSTATIC:
	{
		HDC hdc = (HDC)wParam;
		SetTextColor(hdc, AppStyles::GetThemeColors(currentTheme).titleText);
		SetBkMode(hdc, TRANSPARENT);
		return (LRESULT)CreateSolidBrush(AppStyles::GetThemeColors(currentTheme).windowBg);
	}

	case WM_DESTROY:
		DeleteObject(hTitleFont);
		if (hButtonFont) DeleteObject(hButtonFont);
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == ID_ABOUT_CLOSE)
		{
			EndDialog(hDlg, ID_ABOUT_CLOSE);
			return TRUE;
		}
		break;

	case WM_CLOSE:
		EndDialog(hDlg, ID_ABOUT_CLOSE);
		return TRUE;
	}
	return FALSE;
}

#pragma endregion


#pragma region Events Dialog

void ShowEventsDialog(HWND parentWnd, HINSTANCE hInst, int theme)
{
	EventsManagerControl::EventsDialog::Show(parentWnd, hInst, theme);
}

#pragma endregion

#pragma endregion
