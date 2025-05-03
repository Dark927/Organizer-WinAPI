#pragma once
#include <windows.h>
#include "Event.h"
#include "EventManager.h"
#include "UIHelpers.h"
#include "AppStyles.h"

class EventsDialog
{
public:
    static void Show(HWND parent, HINSTANCE hInst, int theme);

private:
    static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static void InitializeControls(HWND hDlg);
    static void LoadEventsToList(HWND hDlg);
    static void UpdateEventDetails(HWND hDlg, int selectedIndex = -1);
    static void SetAlarmForSelectedEvent(HWND hDlg);
    static void ShowEventStats(HWND hParent);
    static std::wstring FormatEventDetails(const Event& event);

    static int currentTheme;
    static HFONT hFont;
    static HWND hCurrentDialog;
    static bool autoUpdateEnabled;
    static int updateIntervalMinutes;
};