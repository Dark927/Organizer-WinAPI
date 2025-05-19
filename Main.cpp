// >> Includes

#include <windows.h>
#include "resource.h"
#include "DialogHandler.h"
#include "ContactBookDialog.h"

// >> Global variables

HINSTANCE hInst;
LPCTSTR szWindowClass = _T("QWERTY");
LPCTSTR szTitle = _T("Organizer");
WNDPROC OriginalTitleProc = NULL;

HWND hTitle;
const char* mainTitle = "ORGANIZER";
bool themeInitialized = false;
int currentTheme = THEME_LIGHT;

// >> Functions Prototypes 

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
void CreateMenuPopups(HMENU hMenu);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK TitleProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void CreateMainTitle(HWND& hWnd, int& clientWidth);
void CreateMainButtons(int clientHeight, HWND& hWnd, int clientWidth);
void CreateMenuPopups(HMENU hMenu);


// >> Main Program Entry
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	// Initialize common controls (required for DateTimePicker)
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_DATE_CLASSES | ICC_WIN95_CLASSES | ICC_USEREX_CLASSES;
	InitCommonControlsEx(&icex);

	if (!LoadLibrary(TEXT("riched20.dll")))
	{
		MessageBoxW(NULL, L"Failed to load RichEdit control", L"Error", MB_ICONERROR);
		return FALSE;
	}

	// Register window class
	MyRegisterClass(hInstance);

	// Initialize the application window
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	// Main message loop
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

// Register window class
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
	wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
	hInst = hInstance;

	// Create main window with a modern look
	hWnd = CreateWindow(szWindowClass, szTitle,
		WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX),	// Remove resizable border and maximize box
		CW_USEDEFAULT, CW_USEDEFAULT, 450, 500,
		NULL, NULL, hInstance, NULL);

	if (!hWnd) return FALSE;

	// Set window background color (light gray)
	HBRUSH hBackground = CreateSolidBrush(RGB(240, 240, 245));
	SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)hBackground);

	// Get client area for centering
	RECT rcClient;
	GetClientRect(hWnd, &rcClient);
	int clientWidth = rcClient.right;
	int clientHeight = rcClient.bottom;

	// ===== TITLE ===== //
	CreateMainTitle(hWnd, clientWidth);

	// Add subtle shadow effect under title
	HWND hTitleShadow = CreateWindow(_T("STATIC"), NULL, WS_VISIBLE | WS_CHILD | SS_ETCHEDHORZ,
		50,                   // x position
		85,                   // y position (under title)
		clientWidth - 100,    // width
		1,                    // height (1 pixel for line)
		hWnd,
		NULL,                 // No menu handle
		hInst, NULL
	);

	// ===== BUTTONS ===== //
	CreateMainButtons(clientHeight, hWnd, clientWidth);

	// ===== MENU ===== //
	HMENU hMenu = CreateMenu();
	CreateMenuPopups(hMenu);
	SetMenu(hWnd, hMenu);
	UIHelpers::ApplyTheme(hWnd, currentTheme, true);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	return TRUE;
}

void CreateMenuPopups(HMENU hMenu)
{
	// View menu
	HMENU hViewMenu = CreatePopupMenu();

	// Themes submenu
	HMENU hThemeMenu = CreatePopupMenu();
	AppendMenu(hThemeMenu, MF_STRING, THEME_LIGHT, _T("Light Theme"));
	AppendMenu(hThemeMenu, MF_STRING, THEME_DARK, _T("Dark Theme"));
	AppendMenu(hThemeMenu, MF_STRING, THEME_SYSTEM, _T("System Theme"));
	AppendMenu(hThemeMenu, MF_STRING, THEME_CLASSIC, _T("Classic Theme"));
	AppendMenu(hThemeMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(hThemeMenu, MF_STRING, THEME_BLUE, _T("Blue Theme"));
	AppendMenu(hThemeMenu, MF_STRING, THEME_GREEN, _T("Green Theme"));
	AppendMenu(hThemeMenu, MF_STRING, THEME_PURPLE, _T("Purple Theme"));
	AppendMenu(hThemeMenu, MF_STRING, THEME_HIGHCONTRAST, _T("High Contrast"));

	AppendMenu(hViewMenu, MF_POPUP, (UINT_PTR)hThemeMenu, _T("Themes"));
	AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hViewMenu, _T("View"));

	// Help menu
	HMENU hHelpMenu = CreatePopupMenu();
	AppendMenu(hHelpMenu, MF_STRING, CONTEXT_MENU_ABOUT_ID, _T("About"));
	AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hHelpMenu, _T("Help"));
}

void CreateMainButtons(int clientHeight, HWND& hWnd, int clientWidth)
{
	int buttonWidth = 180;
	int buttonHeight = 45;
	int buttonSpacing = 20;
	int totalButtonsHeight = (buttonHeight * 2) + buttonSpacing;
	int buttonsStartY = (clientHeight - totalButtonsHeight) / 2 - 50; // Offset from center

	if (!themeInitialized)
	{
		SetWindowTheme(hWnd, L"Explorer", NULL); // Enable visual styles
	}

	// Create buttons with owner-draw style
	HWND hEventsButton = CreateWindow(_T("BUTTON"), _T("Менеджер подій"),
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_OWNERDRAW,
		(clientWidth - buttonWidth) / 2, buttonsStartY, buttonWidth, buttonHeight,
		hWnd, (HMENU)EVENTS_BUTTON_ID, hInst, NULL);

	HWND hPhoneBookButton = CreateWindow(_T("BUTTON"), _T("Телефонна книга"),
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_OWNERDRAW,
		(clientWidth - buttonWidth) / 2, buttonsStartY + buttonHeight + buttonSpacing,
		buttonWidth, buttonHeight, hWnd, (HMENU)CONTACTS_BUTTON_ID, hInst, NULL);

	SendMessage(hEventsButton, WM_SETFONT, (WPARAM)AppStyles::GetDefaultButtonFont(), TRUE);
	SendMessage(hPhoneBookButton, WM_SETFONT, (WPARAM)AppStyles::GetDefaultButtonFont(), TRUE);

}

void CreateMainTitle(HWND& hWnd, int& clientWidth)
{
	int titleHeight = 72;  // Increased height for larger text
	hTitle = CreateWindow(_T("STATIC"), _T(mainTitle),
		WS_VISIBLE | WS_CHILD | SS_CENTER | SS_NOTIFY,  // Removed SS_OWNERDRAW
		0, 20, clientWidth, titleHeight, hWnd, NULL, hInst, NULL);

	// Create larger font - using 42pt for bigger text
	HDC hdc = GetDC(hWnd);
	int fontHeight = -MulDiv(24, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	ReleaseDC(hWnd, hdc);

	HFONT hTitleFont = AppStyles::GetDefaultTitleFont(fontHeight);

	SendMessage(hTitle, WM_SETFONT, (WPARAM)hTitleFont, TRUE);

	// Subclass the title control
	OriginalTitleProc = (WNDPROC)SetWindowLongPtr(hTitle, GWLP_WNDPROC, (LONG_PTR)TitleProc);
}

// Window procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case CONTEXT_MENU_ABOUT_ID:
			ShowAboutDialog(hWnd, hInst, currentTheme);
			break;

		case EVENTS_BUTTON_ID:
			ShowEventsDialog(hWnd, hInst, currentTheme);
			break;

		case CONTACTS_BUTTON_ID:
			ContactBookControl::ContactBookDialog::Show(hWnd, hInst, currentTheme);
			break;

		case THEME_LIGHT: case THEME_DARK: case THEME_SYSTEM:
		case THEME_CLASSIC: case THEME_BLUE: case THEME_GREEN:
		case THEME_PURPLE: case THEME_HIGHCONTRAST:
			currentTheme = wParam;
			UIHelpers::ApplyTheme(hWnd, LOWORD(wParam), true);
			break;
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_DRAWITEM:
		if (UIHelpers::DrawThemedButton((LPDRAWITEMSTRUCT)lParam, currentTheme))
		{
			return TRUE;
		}
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}


LRESULT CALLBACK TitleProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		// Get current font
		HFONT hFont = (HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0);
		HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

		AppStyles::ThemeColors colors = AppStyles::GetThemeColors(currentTheme);

		// Set colors
		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, colors.titleText);

		// Draw text with padding
		RECT rc;
		GetClientRect(hWnd, &rc);
		rc.top += 5;  // Add some padding

		DrawText(hdc, _T("ORGANIZER"), -1, &rc,
			DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		// Cleanup
		SelectObject(hdc, hOldFont);
		EndPaint(hWnd, &ps);
		return 0;
	}
	}

	return CallWindowProc(OriginalTitleProc, hWnd, msg, wParam, lParam);
}