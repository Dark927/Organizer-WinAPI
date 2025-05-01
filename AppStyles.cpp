
#include "AppStyles.h"

namespace AppStyles
{
	ThemeColors GetThemeColors(int theme)
	{
		ThemeColors colors;

		switch (theme)
		{
		case THEME_LIGHT:
			colors.windowBg = RGB(240, 240, 245);
			colors.titleBg = RGB(240, 240, 245);
			colors.titleText = RGB(50, 50, 80);
			colors.buttonBg = RGB(70, 130, 200);
			colors.buttonEdge = RGB(50, 100, 180);
			colors.buttonText = RGB(255, 255, 255);
			break;

		case THEME_DARK:
			colors.windowBg = RGB(32, 32, 32);
			colors.titleBg = RGB(32, 32, 32);
			colors.titleText = RGB(220, 220, 220);
			colors.buttonBg = RGB(60, 60, 80);
			colors.buttonEdge = RGB(40, 40, 60);
			colors.buttonText = RGB(240, 240, 240);
			break;

		case THEME_SYSTEM:
			colors.windowBg = GetSysColor(COLOR_WINDOW);
			colors.titleBg = GetSysColor(COLOR_WINDOW);
			colors.titleText = GetSysColor(COLOR_WINDOWTEXT);
			colors.buttonBg = GetSysColor(COLOR_BTNFACE);
			colors.buttonEdge = GetSysColor(COLOR_BTNSHADOW);
			colors.buttonText = GetSysColor(COLOR_BTNTEXT);
			break;

		case THEME_CLASSIC:
			colors.windowBg = GetSysColor(COLOR_BTNFACE);
			colors.titleBg = GetSysColor(COLOR_BTNFACE);
			colors.titleText = GetSysColor(COLOR_BTNTEXT);
			colors.buttonBg = GetSysColor(COLOR_BTNFACE);
			colors.buttonEdge = GetSysColor(COLOR_BTNSHADOW);
			colors.buttonText = GetSysColor(COLOR_BTNTEXT);
			break;

		case THEME_BLUE:
			colors.windowBg = RGB(200, 220, 255);
			colors.titleBg = RGB(180, 200, 240);
			colors.titleText = RGB(20, 40, 100);
			colors.buttonBg = RGB(100, 140, 220);
			colors.buttonEdge = RGB(80, 120, 200);
			colors.buttonText = RGB(255, 255, 255);
			break;

		case THEME_GREEN:
			colors.windowBg = RGB(220, 255, 220);
			colors.titleBg = RGB(200, 240, 200);
			colors.titleText = RGB(0, 80, 0);
			colors.buttonBg = RGB(80, 160, 80);
			colors.buttonEdge = RGB(60, 140, 60);
			colors.buttonText = RGB(255, 255, 255);
			break;

		case THEME_PURPLE:
			colors.windowBg = RGB(240, 220, 255);
			colors.titleBg = RGB(220, 200, 240);
			colors.titleText = RGB(80, 0, 100);
			colors.buttonBg = RGB(160, 80, 180);
			colors.buttonEdge = RGB(140, 60, 160);
			colors.buttonText = RGB(255, 255, 255);
			break;

		case THEME_HIGHCONTRAST:
			colors.windowBg = RGB(0, 0, 0);
			colors.titleBg = RGB(0, 0, 0);
			colors.titleText = RGB(255, 255, 255);
			colors.buttonBg = RGB(0, 0, 0);
			colors.buttonEdge = RGB(255, 255, 255);
			colors.buttonText = RGB(255, 255, 255);
			break;

		default:
			return GetThemeColors(THEME_SYSTEM);
		}

		return colors;
	}


	HFONT GetDefaultTitleFont(int size)
	{
		return CreateFont(
			size, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
			DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
			CLEARTYPE_QUALITY, VARIABLE_PITCH, _T("Georgia"));
	}

	HFONT GetDefaultButtonFont(int size)
	{
		return CreateFont(size, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE,
			DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			CLEARTYPE_QUALITY, DEFAULT_PITCH, _T("Segoe UI"));
	}
}