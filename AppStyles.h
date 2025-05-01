#pragma once

#include <windows.h>
#include <uxtheme.h> 
#include <tchar.h>                      // Required for TCHAR support
#pragma comment(lib, "uxtheme.lib")

namespace AppStyles
{
#define THEME_LIGHT 201
#define THEME_DARK 202
#define THEME_SYSTEM 203
#define THEME_CLASSIC 204
#define THEME_BLUE 205
#define THEME_GREEN 206
#define THEME_PURPLE 207
#define THEME_HIGHCONTRAST 208

	// Global structure to hold theme colors
	struct ThemeColors
	{
		COLORREF windowBg;
		COLORREF titleBg;
		COLORREF titleText;
		COLORREF buttonBg;
		COLORREF buttonEdge;
		COLORREF buttonText;
	};

	struct WindowThemeData
	{
		int theme;
		COLORREF bgColor;
		COLORREF textColor;
	};

	ThemeColors GetThemeColors(int theme);
	HFONT GetDefaultTitleFont(int size = 14);
	HFONT GetDefaultButtonFont(int size = 24);
}
