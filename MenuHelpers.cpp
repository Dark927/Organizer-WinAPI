#include "MenuHelpers.h"
#include "resource.h"

bool MenuHelpers::HandleMenuCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
	case IDM_FILE_LOAD:
	case IDM_FILE_SAVE:
	case IDM_FILE_SAVEAS:
	case IDM_FILE_EXIT:
	case IDM_HELP_INSTRUCTIONS:
		// Just return true to indicate these commands are handled
		// Actual implementation would be in the specific dialog
		return true;
	}
	return false;
}