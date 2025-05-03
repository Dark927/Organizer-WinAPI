#pragma once
#include <windows.h>
#include "Event.h"
#include "UIHelpers.h"
#include "AppStyles.h"
#include <commctrl.h>

class EventInputDialog
{
public:
    static const int MAX_NAME_LENGTH = 64;
    static const int MAX_DESC_LENGTH = 1500;
    static const int MAX_NOTES_LENGTH = 512;

    static bool Show(HWND parent, Event& event, int theme);

private:
    static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static void InitializeControls(HWND hDlg, Event& event);
    static bool GetInput(HWND hDlg, Event& event);
    static void UpdateCharCount(HWND hDlg, int editId, int countId, int maxLength);
    static LRESULT CALLBACK CheckBoxSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

    static int currentTheme;
    static HFONT hFont;
    static Event* pEvent;
};