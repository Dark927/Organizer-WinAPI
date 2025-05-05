
#include "HotkeysHandler.h"
#include "resource.h"

namespace HotkeysControl
{
	void UnregisterDefaultMenuHotkeys(HWND hDlg)
	{
		UnregisterHotKey(hDlg, IDM_HELP_INSTRUCTIONS);
		UnregisterHotKey(hDlg, IDM_FILE_LOAD);
		UnregisterHotKey(hDlg, IDM_FILE_SAVE);
		UnregisterHotKey(hDlg, IDM_FILE_SAVEAS);
		UnregisterHotKey(hDlg, IDM_FILE_EXIT);
	}

	void RegisterDefaultMenuHotkeys(HWND hDlg)
	{
		RegisterHotKey(hDlg, IDM_HELP_INSTRUCTIONS, MOD_NOREPEAT, VK_F1);
		RegisterHotKey(hDlg, IDM_FILE_LOAD, MOD_CONTROL | MOD_NOREPEAT, 'O');
		RegisterHotKey(hDlg, IDM_FILE_SAVE, MOD_CONTROL | MOD_NOREPEAT, 'S');
		RegisterHotKey(hDlg, IDM_FILE_SAVEAS, MOD_CONTROL | MOD_SHIFT | MOD_NOREPEAT, 'S');
		RegisterHotKey(hDlg, IDM_FILE_EXIT, MOD_ALT | MOD_NOREPEAT, VK_F4);
	}
}