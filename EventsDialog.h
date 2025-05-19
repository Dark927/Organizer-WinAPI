#pragma once
#include <windows.h>
#include "Event.h"
#include "EventManager.h"
#include "UIHelpers.h"
#include "AppStyles.h"

namespace EventsManagerControl
{
	class EventsDialog
	{
	public:
		static void Show(HWND parent, HINSTANCE hInst, int theme);

	private:
		static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
		static INT_PTR CALLBACK InstructionsProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
		static void ProcessCreateEvent(HWND hDlg);
		
		static void InitializeEventsManagerDialog(LPARAM lParam, HWND hDlg);
		static void InitializeControls(HWND hDlg);

		static void LoadEventsToList(HWND hDlg);
		static void UpdateEventDetails(HWND hDlg, int selectedIndex = -1);
		static void UpdateEventsListState(HWND hDlg);
		static void SetEventsUpdateInterval(HWND hDlg, int intervalInSec);
		static std::wstring FormatEventDetails(const Event& event);
		
		static void LoadEventsFromFile(HWND hDlg);
		static void SaveEventsToFileAs(HWND hDlg);
		
		static bool IsSelectedDateTimeInPast(HWND hDlg);

		static void ShowInstructions(HWND hDlg);
		static void SaveEventsToFile(HWND hDlg);
		static void CheckAndShowAlarms(HWND hDlg);
		static void Dispose(HWND hDlg);

		static bool ShouldShowNotification(const AlarmNotification& alarm);
		static void ShowNotification(HWND hDlg, const AlarmNotification& alarm);


		static int currentTheme;
		static HFONT hFont;
		static HWND hCurrentDialog;
		static bool autoUpdateEnabled;
		static int updateIntervalSeconds;
	};
}