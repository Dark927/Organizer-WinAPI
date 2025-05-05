#define UNICODE
#define _UNICODE

#include "EventsDialog.h"
#include <algorithm>
#include <iomanip>
#include <sstream>
#include "resource.h"
#include "Utils.h"
#include "HotkeysHandler.h"
#include "EventInputDialog.h"
#include <richedit.h>
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")

namespace EventsManagerControl
{
#define SCROLL_TIMER_ID 10101
#define EVENTS_UPDATE_TIMER_ID 10102

	int EventsDialog::currentTheme = 0;
	HFONT EventsDialog::hFont = nullptr;
	HWND EventsDialog::hCurrentDialog = nullptr;
	bool EventsDialog::autoUpdateEnabled = true;
	int EventsDialog::updateIntervalSeconds = 5;
	static HBRUSH hEditBackgroundBrush = NULL;
	std::wstring lastFile;

	EventsManager* eventsManager = nullptr;


#pragma region Main Dialog Logic

	void EventsDialog::Show(HWND parent, HINSTANCE hInst, int theme)
	{
		eventsManager = new EventsManager();
		currentTheme = theme;
		DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_EVENTS_DIALOG), parent,
			DialogProc, (LPARAM)theme);
	}

	INT_PTR CALLBACK EventsDialog::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_INITDIALOG:
		{
			InitializeEventsManagerDialog(lParam, hDlg);
			return TRUE;
		}

		case WM_ACTIVATE:
			if (wParam == WA_INACTIVE)
			{
				HotkeysControl::UnregisterDefaultMenuHotkeys(hDlg);
			}
			else
			{
				HotkeysControl::RegisterDefaultMenuHotkeys(hDlg);
			}
			break;

		case WM_TIMER:
		{
			if (wParam == EVENTS_UPDATE_TIMER_ID)
			{
				UpdateEventsListState(hDlg);
				CheckAndShowAlarms(hDlg);
			}

			if (wParam == SCROLL_TIMER_ID)
			{
				KillTimer(hDlg, SCROLL_TIMER_ID);
				HWND hRichEdit = GetDlgItem(hDlg, IDC_EVENT_DETAILS_RICHEDIT);

				SendMessage(hRichEdit, EM_SETSEL, 0, 0);
				SendMessage(hRichEdit, EM_SCROLLCARET, 0, 0);
				SendMessage(hRichEdit, WM_VSCROLL, SB_TOP, 0);
			}
		}
		break;

		case WM_DRAWITEM:
			if (UIHelpers::DrawThemedButton((LPDRAWITEMSTRUCT)lParam, currentTheme))
			{
				return TRUE;
			}
			break;

		case WM_CTLCOLORDLG:
		case WM_CTLCOLORSTATIC:
		case WM_CTLCOLORBTN:
		{
			HDC hdc = (HDC)wParam;
			SetTextColor(hdc, AppStyles::GetThemeColors(currentTheme).titleText);
			SetBkMode(hdc, TRANSPARENT);
			return (LRESULT)CreateSolidBrush(AppStyles::GetThemeColors(currentTheme).windowBg);
		}

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{

			case IDM_FILE_LOAD:
				LoadEventsFromFile(hDlg);
				break;

			case IDM_FILE_SAVE:
				SaveEventsToFile(hDlg);
				break;

			case IDM_FILE_SAVEAS:
				SaveEventsToFileAs(hDlg);
				break;

			case IDM_FILE_EXIT:
				SendMessage(hDlg, WM_CLOSE, 0, 0);
				break;

			case IDM_HELP_INSTRUCTIONS:
				ShowInstructions(hDlg);
				break;

			case ID_CREATE_EVENT_BTN:
			{
				if (IsSelectedDateTimeInPast(hDlg))
				{
					MessageBoxW(hDlg, L"Неможливо додати подію в минуле",
						L"Попередження", MB_ICONWARNING);
					return TRUE;
				}

				ProcessCreateEvent(hDlg);
				return TRUE;
			}

			case ID_DELETE_EVENT_BTN:
			{
				int selected = SendDlgItemMessage(hDlg, IDC_EVENTS_LIST, LB_GETCURSEL, 0, 0);
				if (selected != LB_ERR)
				{
					if (MessageBoxW(hDlg, L"Ви впевнені, що хочете видалити цю подію?",
						L"Підтвердження", MB_YESNO | MB_ICONQUESTION) == IDYES)
					{
						eventsManager->DeleteEvent(selected);
						LoadEventsToList(hDlg);
					}
				}
				else
				{
					MessageBoxW(hDlg, L"Будь ласка, виберіть подію для видалення", L"Попередження", MB_ICONWARNING);
				}
				return TRUE;
			}

			case IDC_EVENT_EDIT_BTN:
			{
				HWND hList = GetDlgItem(hDlg, IDC_EVENTS_LIST);
				int selectedIndex = (int)SendMessage(hList, LB_GETCURSEL, 0, 0);

				if (selectedIndex != LB_ERR)
				{
					const Event* allEvents = eventsManager->GetCurrentEvents();
					Event eventToEdit = allEvents[selectedIndex];

					if (EventInputDialog::Show(hDlg, eventToEdit, currentTheme, EventInputDialog::Mode::Edit))
					{
						eventsManager->UpdateEvent(selectedIndex, eventToEdit);
						LoadEventsToList(hDlg);

						// Reselect the edited item
						SendMessage(hList, LB_SETCURSEL, selectedIndex, 0);
						UpdateEventDetails(hDlg, selectedIndex);
					}
				}
				else
				{
					MessageBoxW(hDlg, L"Будь ласка, виберіть подію для редагування", L"Попередження", MB_ICONWARNING);
				}
				return TRUE;
			}

			case IDC_EVENT_DROP_BTN:
			{
				eventsManager->DiscardChanges();
				LoadEventsToList(hDlg);
				return TRUE;
			}

			case IDC_EVENT_UPDATE_BTN:
			{
				eventsManager->UpdateEventsStatus();
				eventsManager->ProcessRecurringEvents();
				LoadEventsToList(hDlg);
				CheckAndShowAlarms(hDlg);
				return TRUE;
			}

			case IDC_EVENT_CLEAR_PAST_BTN:
			{
				if (MessageBoxW(hDlg, L"Видалити всі минулі події?",
					L"Підтвердження", MB_YESNO | MB_ICONQUESTION) == IDYES)
				{
					eventsManager->ClearPastDueEvents();
					LoadEventsToList(hDlg);
				}
				return TRUE;
			}

			case IDC_EVENT_SORT_COMBO:
			{
				if (HIWORD(wParam) == CBN_SELCHANGE)
				{
					int sel = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
					eventsManager->SortEvents(static_cast<EventsManager::SortCriteria>(sel));
					LoadEventsToList(hDlg);
				}
				return TRUE;
			}

			case IDC_EVENTS_LIST:
				if (HIWORD(wParam) == LBN_SELCHANGE)
				{
					int selected = SendMessage((HWND)lParam, LB_GETCURSEL, 0, 0);
					UpdateEventDetails(hDlg, selected);
					return TRUE;
				}
				break;

			case IDC_AUTOUPDATE_CHECK:
				if (HIWORD(wParam) == BN_CLICKED)
				{
					autoUpdateEnabled = IsDlgButtonChecked(hDlg, IDC_AUTOUPDATE_CHECK) == BST_CHECKED;

					// Enable/disable interval controls
					EnableWindow(GetDlgItem(hDlg, IDC_UPDATE_INTERVAL_EDIT), autoUpdateEnabled);
					EnableWindow(GetDlgItem(hDlg, IDC_UPDATE_INTERVAL_SPIN), autoUpdateEnabled);
					EnableWindow(GetDlgItem(hDlg, IDC_APPLY_UPDATE_SETTINGS_BTN), autoUpdateEnabled);

					// Start/stop timer
					if (autoUpdateEnabled)
					{
						SetEventsUpdateInterval(hDlg, updateIntervalSeconds);
					}
					else
					{
						KillTimer(hDlg, EVENTS_UPDATE_TIMER_ID);
					}
				}
				return TRUE;

			case IDC_APPLY_UPDATE_SETTINGS_BTN:
				if (autoUpdateEnabled)
				{
					BOOL success;
					UINT newInterval = GetDlgItemInt(hDlg, IDC_UPDATE_INTERVAL_EDIT, &success, FALSE);
					if (success && newInterval >= 1 && newInterval <= 3600)
					{
						updateIntervalSeconds = newInterval;
						SetEventsUpdateInterval(hDlg, updateIntervalSeconds);
						MessageBoxW(hDlg, L"Налаштування оновлення збережено", L"Успіх", MB_ICONINFORMATION);
					}
					else
					{
						MessageBoxW(hDlg, L"Будь ласка, введіть коректний інтервал (1-3600 секунд)", L"Помилка", MB_ICONERROR);
					}
				}
				return TRUE;

			case IDCANCEL:
				EndDialog(hDlg, IDCANCEL);
				return TRUE;
			}
			break;

		case WM_NOTIFY:
		{
			LPNMHDR nmhdr = (LPNMHDR)lParam;
			if (nmhdr->code == MCN_SELCHANGE || nmhdr->code == DTN_DATETIMECHANGE)
			{
				bool isPast = IsSelectedDateTimeInPast(hDlg);

				if (isPast)
				{
					// Optional: Show warning text somewhere
					SetDlgItemText(hDlg, IDC_EVENT_TIME_WARNING,
						L"Попередження: обрана дата/час вже минули");
				}
				else
				{
					SetDlgItemText(hDlg, IDC_EVENT_TIME_WARNING, L"");
				}
				return TRUE;
			}
			break;
		}

		case WM_HOTKEY:
		{
			SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(wParam, 0), 0);
			return TRUE;
		}

		case WM_CLOSE:
			KillTimer(hDlg, 1);
			EndDialog(hDlg, IDCANCEL);
			return TRUE;

		case WM_DESTROY:
			Dispose(hDlg);
			break;
		}
		return FALSE;
	}

	void EventsDialog::ProcessCreateEvent(HWND hDlg)
	{
		Event newEvent;

		// Get selected date from calendar
		SYSTEMTIME selectedDate;
		MonthCal_GetCurSel(GetDlgItem(hDlg, IDC_EVENTS_CALENDAR), &selectedDate);

		// Get selected time from time picker
		SYSTEMTIME selectedTime;
		DateTime_GetSystemtime(GetDlgItem(hDlg, IDC_EVENTS_TIME_PICKER), &selectedTime);

		// Combine date and time
		newEvent.targetDateTime = selectedDate;
		newEvent.targetDateTime.wHour = selectedTime.wHour;
		newEvent.targetDateTime.wMinute = selectedTime.wMinute;
		newEvent.targetDateTime.wSecond = 0;
		newEvent.targetDateTime.wMilliseconds = 0;
		newEvent.targetDateTime.wDayOfWeek = 0;

		if (EventInputDialog::Show(hDlg, newEvent, currentTheme))
		{
			eventsManager->AddEvent(newEvent);
			LoadEventsToList(hDlg);
		}
	}

	bool EventsDialog::IsSelectedDateTimeInPast(HWND hDlg)
	{
		SYSTEMTIME selectedDate, selectedTime, currentTime;

		// Get selected date from calendar
		MonthCal_GetCurSel(GetDlgItem(hDlg, IDC_EVENTS_CALENDAR), &selectedDate);

		// Get selected time from time picker
		DateTime_GetSystemtime(GetDlgItem(hDlg, IDC_EVENTS_TIME_PICKER), &selectedTime);

		// Combine date and time
		SYSTEMTIME selectedDateTime = selectedDate;
		selectedDateTime.wHour = selectedTime.wHour;
		selectedDateTime.wMinute = selectedTime.wMinute;
		selectedDateTime.wSecond = 0;
		selectedDateTime.wMilliseconds = 0;

		// Compare
		return Utils::IsDateTimeInPast(selectedDateTime);
	}

#pragma endregion


#pragma region Init & Dispose

	void EventsDialog::InitializeEventsManagerDialog(LPARAM lParam, HWND hDlg)
	{
		currentTheme = (int)lParam;
		hCurrentDialog = hDlg;
		eventsManager->LoadFromFile(lastFile);
		InitializeControls(hDlg);
		LoadEventsToList(hDlg);
		SetEventsUpdateInterval(hDlg, updateIntervalSeconds);

		UpdateEventsListState(hDlg);
		HotkeysControl::RegisterDefaultMenuHotkeys(hDlg);
	}

	void EventsDialog::InitializeControls(HWND hDlg)
	{
		hFont = AppStyles::GetDefaultButtonFont();

		HWND hRichEdit = GetDlgItem(hDlg, IDC_EVENT_DETAILS_RICHEDIT);

		// Set background color
		SendMessage(hRichEdit, EM_SETBKGNDCOLOR, 0,
			AppStyles::GetThemeColors(currentTheme).windowBg);

		// Initialize time picker
		HWND hTimePicker = GetDlgItem(hDlg, IDC_EVENTS_TIME_PICKER);
		DateTime_SetFormat(hTimePicker, L"HH':'mm");

		// Set current time (default to now + 1 hour)
		SYSTEMTIME st;
		GetLocalTime(&st);
		st.wHour = (st.wHour + 1) % 24;
		DateTime_SetSystemtime(hTimePicker, GDT_VALID, &st);

		// RichEdit formatting
		CHARFORMAT2 cf;
		memset(&cf, 0, sizeof(CHARFORMAT2));
		cf.cbSize = sizeof(CHARFORMAT2);
		cf.dwMask = CFM_COLOR | CFM_FACE | CFM_SIZE | CFM_CHARSET;
		cf.crTextColor = AppStyles::GetThemeColors(currentTheme).titleText;
		cf.yHeight = 200; // 10pt
		cf.bCharSet = DEFAULT_CHARSET;
		wcscpy_s(cf.szFaceName, LF_FACESIZE, L"Segoe UI");
		SendMessage(hRichEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);

		// Initialize owner-draw buttons
		UIHelpers::InitOwnerDrawButton(hDlg, ID_CREATE_EVENT_BTN, hFont);
		UIHelpers::InitOwnerDrawButton(hDlg, ID_DELETE_EVENT_BTN, hFont);
		UIHelpers::InitOwnerDrawButton(hDlg, IDC_EVENT_EDIT_BTN, hFont);
		UIHelpers::InitOwnerDrawButton(hDlg, IDC_EVENT_STATS_BTN, hFont);
		UIHelpers::InitOwnerDrawButton(hDlg, IDC_EVENT_UPDATE_BTN, hFont);
		UIHelpers::InitOwnerDrawButton(hDlg, IDC_EVENT_CLEAR_PAST_BTN, hFont);
		UIHelpers::InitOwnerDrawButton(hDlg, IDC_APPLY_UPDATE_SETTINGS_BTN, hFont);
		UIHelpers::InitOwnerDrawButton(hDlg, IDCANCEL, hFont);

		// Initialize sort combo
		HWND hSortCombo = GetDlgItem(hDlg, IDC_EVENT_SORT_COMBO);
		SendMessage(hSortCombo, CB_ADDSTRING, 0, (LPARAM)L"За назвою");
		SendMessage(hSortCombo, CB_ADDSTRING, 0, (LPARAM)L"За датою");
		SendMessage(hSortCombo, CB_ADDSTRING, 0, (LPARAM)L"За важливістю");
		SendMessage(hSortCombo, CB_ADDSTRING, 0, (LPARAM)L"За датою створення");
		SendMessage(hSortCombo, CB_SETCURSEL, 0, 0);

		// Initialize listbox with horizontal scrolling
		HWND hListBox = GetDlgItem(hDlg, IDC_EVENTS_LIST);

		// Set tab stops (optional, helps with alignment)
		const int tabStops[] = { 150, 300 }; // Adjust as needed
		SendMessage(hListBox, LB_SETTABSTOPS, 2, (LPARAM)tabStops);

		// Set horizontal extent to 0 initially (will be updated when items are added)
		SendMessage(hListBox, LB_SETHORIZONTALEXTENT, 0, 0);

		// Set font for the listbox
		SendMessage(hListBox, WM_SETFONT, (WPARAM)hFont, TRUE);

		// Initialize auto-update controls
		CheckDlgButton(hDlg, IDC_AUTOUPDATE_CHECK, autoUpdateEnabled ? BST_CHECKED : BST_UNCHECKED);

		// Initialize update interval spinner (1-3600 seconds)
		HWND hSpin = GetDlgItem(hDlg, IDC_UPDATE_INTERVAL_SPIN);
		SendMessage(hSpin, UDM_SETRANGE32, 1, 3600); // 1 second to 1 hour
		SendMessage(hSpin, UDM_SETPOS32, 0, updateIntervalSeconds);
		SetDlgItemInt(hDlg, IDC_UPDATE_INTERVAL_EDIT, updateIntervalSeconds, FALSE);

		// Disable interval controls if auto-update is off
		EnableWindow(GetDlgItem(hDlg, IDC_UPDATE_INTERVAL_EDIT), autoUpdateEnabled);
		EnableWindow(GetDlgItem(hDlg, IDC_UPDATE_INTERVAL_SPIN), autoUpdateEnabled);
		EnableWindow(GetDlgItem(hDlg, IDC_APPLY_UPDATE_SETTINGS_BTN), autoUpdateEnabled);

		UIHelpers::ApplyTheme(hDlg, currentTheme);
	}

	void EventsDialog::Dispose(HWND hDlg)
	{
		if (hEditBackgroundBrush)
		{
			DeleteObject(hEditBackgroundBrush);
			hEditBackgroundBrush = NULL;
		}
		HotkeysControl::UnregisterDefaultMenuHotkeys(hDlg);
		delete eventsManager;
		eventsManager = nullptr;
		KillTimer(hDlg, EVENTS_UPDATE_TIMER_ID);
		KillTimer(hDlg, SCROLL_TIMER_ID);
		if (hFont) DeleteObject(hFont);
		hCurrentDialog = nullptr;
	}
	
#pragma endregion


#pragma region Events list & Event info Management

	void EventsDialog::LoadEventsToList(HWND hDlg)
	{
		HWND hList = GetDlgItem(hDlg, IDC_EVENTS_LIST);

		// Store current selection info
		int prevSelectedIndex = (int)SendMessage(hList, LB_GETCURSEL, 0, 0);
		int prevEventIndex = LB_ERR;
		if (prevSelectedIndex != LB_ERR)
		{
			prevEventIndex = (int)SendMessage(hList, LB_GETITEMDATA, prevSelectedIndex, 0);
		}

		// Clear the list
		SendMessage(hList, LB_RESETCONTENT, 0, 0);

		const Event* allEvents = eventsManager->GetCurrentEvents();
		int eventsCount = eventsManager->GetCurrentEventCount();

		// Get device context for text measurement
		HDC hdc = GetDC(hList);
		HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
		int maxWidth = 0;

		for (int i = 0; i < eventsCount; ++i)
		{
			wchar_t buffer[256];
			wchar_t statusChar = L' ';

			if (allEvents[i].isRecurring && allEvents[i].isPastDue)
				statusChar = L'⦻';
			else if (allEvents[i].isRecurring)
				statusChar = L'⟳';
			else if (allEvents[i].isPastDue)
				statusChar = L'!';
			else
				statusChar = L'•';

			swprintf_s(buffer, L"[%c] %02d/%02d %02d:%02d | %s",
				statusChar,
				allEvents[i].targetDateTime.wDay, allEvents[i].targetDateTime.wMonth,
				allEvents[i].targetDateTime.wHour, allEvents[i].targetDateTime.wMinute,
				allEvents[i].name.c_str());

			// Add to list
			int index = (int)SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)buffer);
			SendMessage(hList, LB_SETITEMDATA, index, (LPARAM)i);

			// Calculate text width
			SIZE size;
			GetTextExtentPoint32(hdc, buffer, (int)wcslen(buffer), &size);
			if (size.cx > maxWidth)
			{
				maxWidth = size.cx;
			}
		}

		// Clean up GDI objects
		SelectObject(hdc, hOldFont);
		ReleaseDC(hList, hdc);

		// Set horizontal scroll extent (add 20px padding)
		SendMessage(hList, LB_SETHORIZONTALEXTENT, maxWidth + 20, 0);

		int tabStops[] = { 100, 150 };
		SendMessage(hList, LB_SETTABSTOPS, 2, (LPARAM)tabStops);

		// Restore selection if possible
		int newCount = (int)SendMessage(hList, LB_GETCOUNT, 0, 0);
		if (newCount > 0)
		{
			int newSelection = 0;

			if (prevEventIndex != LB_ERR)
			{
				// Try to find the previously selected event by its original index
				for (int i = 0; i < newCount; i++)
				{
					int eventIndex = (int)SendMessage(hList, LB_GETITEMDATA, i, 0);
					if (eventIndex == prevEventIndex)
					{
						newSelection = i;
						break;
					}
				}
			}

			SendMessage(hList, LB_SETCURSEL, newSelection, 0);
			UpdateEventDetails(hDlg, (int)SendMessage(hList, LB_GETITEMDATA, newSelection, 0));
		}
		else
		{
			UpdateEventDetails(hDlg, -1);
		}
	}

	void EventsDialog::UpdateEventDetails(HWND hDlg, int selectedIndex)
	{
		HWND hRichEdit = GetDlgItem(hDlg, IDC_EVENT_DETAILS_RICHEDIT);

		// Disable redraw while updating
		SendMessage(hRichEdit, WM_SETREDRAW, FALSE, 0);

		if (selectedIndex == -1)
		{
			SetWindowTextW(hRichEdit, L"Оберіть подію для перегляду деталей");
		}
		else
		{
			const Event* allEvents = eventsManager->GetCurrentEvents();
			int eventsCount = eventsManager->GetCurrentEventCount();

			if (selectedIndex >= 0 && selectedIndex < eventsCount)
			{
				const Event& event = allEvents[selectedIndex];
				std::wstring text = FormatEventDetails(event);
				SetWindowTextW(hRichEdit, text.c_str());
			}
		}

		// Re-enable redraw before scrolling
		SendMessage(hRichEdit, WM_SETREDRAW, TRUE, 0);

		// Force a complete repaint
		InvalidateRect(hRichEdit, NULL, TRUE);
		UpdateWindow(hRichEdit);
		SetTimer(hDlg, SCROLL_TIMER_ID, 50, NULL);
	}


	std::wstring EventsDialog::FormatEventDetails(const Event& event)
	{
		std::wstringstream details;
		details << L"Назва: " << event.name << L"\r";
		details << L"Дата/час події: "
			<< event.targetDateTime.wDay << L"."
			<< event.targetDateTime.wMonth << L"."
			<< event.targetDateTime.wYear << L" "
			<< event.targetDateTime.wHour << L":"
			<< std::setfill(L'0') << std::setw(2) << event.targetDateTime.wMinute << L"\r";

		int filledStars = event.importance;
		int emptyStars = 5 - filledStars;
		std::wstring stars = L"";

		// > Add stars

		for (int i = 0; i < filledStars; ++i)
		{
			stars += L"★";
		}

		for (int i = 0; i < emptyStars; ++i)
		{
			stars += L"☆";
		}

		details << L"Важливість: " << stars << L"\r";
		details << L"Стан: " << (event.isPastDue ? L"Минула" : L"Майбутня") << L"\r";
		details << L"Створено: "
			<< event.creationDate.wDay << L"."
			<< event.creationDate.wMonth << L"."
			<< event.creationDate.wYear << L"\r";
		details << L"Повторювана: " << (event.isRecurring ? L"Так" : L"Ні") << L"\r";
		if (event.isRecurring)
		{
			details << L"Інтервал: кожні " << event.recurrenceInterval << L" днів\r";
		}
		details << L"Сповіщення: " << (event.hasAlarm ? L"Так" : L"Ні");

		details << L"\n\nОпис:\r\n" << event.description << L"\r\n\r\n";
		details << L"Нотатки:\r\n" << event.notes << L"\r\n\r\n";

		return details.str();
	}



	void EventsDialog::UpdateEventsListState(HWND hDlg)
	{
		HWND hList = GetDlgItem(hDlg, IDC_EVENTS_LIST);
		int selectedIndex = (int)SendMessage(hList, LB_GETCURSEL, 0, 0);

		eventsManager->UpdateEventsStatus();
		eventsManager->ProcessRecurringEvents();
		LoadEventsToList(hDlg);

		if (selectedIndex != LB_ERR)
		{
			// Ensure the index is still valid after update
			int itemCount = (int)SendMessage(hList, LB_GETCOUNT, 0, 0);
			if (selectedIndex < itemCount)
			{
				SendMessage(hList, LB_SETCURSEL, selectedIndex, 0);
				UpdateEventDetails(hDlg, selectedIndex);
			}
			else if (itemCount > 0)
			{
				// If previous selection is now invalid, select last item
				SendMessage(hList, LB_SETCURSEL, itemCount - 1, 0);
				UpdateEventDetails(hDlg, itemCount - 1);
			}
		}
	}


	void EventsDialog::SetEventsUpdateInterval(HWND hDlg, int intervalInSec)
	{
		updateIntervalSeconds = intervalInSec;

		// Update UI controls
		SetDlgItemInt(hDlg, IDC_UPDATE_INTERVAL_EDIT, intervalInSec, FALSE);
		SendMessage(GetDlgItem(hDlg, IDC_UPDATE_INTERVAL_SPIN), UDM_SETPOS32, 0, intervalInSec);

		// Manage the timer
		KillTimer(hDlg, EVENTS_UPDATE_TIMER_ID);
		if (autoUpdateEnabled && intervalInSec >= 1)
		{
			SetTimer(hDlg, EVENTS_UPDATE_TIMER_ID, intervalInSec * 1000, nullptr);
		}
	}

#pragma endregion


#pragma region File Load/Save Management

	void EventsDialog::LoadEventsFromFile(HWND hDlg)
	{
		// Initialize the OPENFILENAME structure
		OPENFILENAME ofn = { 0 };
		wchar_t* szFile = Utils::SelectLoadFile(hDlg, ofn);

		// Show the Open dialog
		if (GetOpenFileName(&ofn))
		{
			if (eventsManager->LoadFromFile(szFile))
			{
				LoadEventsToList(hDlg);
				lastFile = szFile;
				MessageBoxW(hDlg, L"Події успішно завантажено", L"Інформація", MB_ICONINFORMATION);
			}
			else
			{
				MessageBoxW(hDlg, L"Помилка при завантаженні подій", L"Помилка", MB_ICONERROR);
			}
		}
	}

	void EventsDialog::SaveEventsToFile(HWND hDlg)
	{
		if (eventsManager->SaveToCurrentFile())
		{
			MessageBoxW(hDlg, L"Події успішно збережено в поточний файл", L"Інформація", MB_ICONINFORMATION);
		}
		else
		{
			int result = MessageBox(hDlg,
				L"Неможливо зберегти в поточний файл, можливо він пошкоджений або не існує. Зберегти в новий файл?",
				L"Збереження контактної книги",
				MB_YESNO | MB_ICONWARNING);

			if (result == IDYES)
			{
				SaveEventsToFileAs(hDlg);
			}
		}
	}

	void EventsDialog::SaveEventsToFileAs(HWND hDlg)
	{
		// Initialize Save As dialog
		OPENFILENAME ofn = { 0 };
		wchar_t* szFile = Utils::SelectSaveFile(hDlg, ofn, L"events.txt");

		// Show the Save As dialog
		if (GetSaveFileName(&ofn))
		{
			if (eventsManager->SaveToFile(szFile))
			{
				MessageBoxW(hDlg, L"Події успішно збережено у файл", L"Інформація", MB_ICONINFORMATION);
			}
			else
			{
				MessageBoxW(hDlg, L"Помилка при збереженні подій у файл", L"Помилка збереження", MB_ICONERROR);
			}
		}
	}

#pragma endregion


#pragma region Alarm Clock

	void EventsDialog::SetAlarmForSelectedEvent(HWND hDlg)
	{
		int selected = SendDlgItemMessage(hDlg, IDC_EVENTS_LIST, LB_GETCURSEL, 0, 0);
		if (selected == LB_ERR)
		{
			MessageBoxW(hDlg, L"Будь ласка, виберіть подію для налаштування сповіщення",
				L"Попередження", MB_ICONWARNING);
			return;
		}

		const Event* allEvents = eventsManager->GetCurrentEvents();
		int eventsCount = eventsManager->GetCurrentEventCount();

		if (selected >= 0 && selected < eventsCount)
		{
			const Event& event = allEvents[selected];
			std::wstring message = L"Сповіщення для події: " + event.name;
			MessageBoxW(hDlg, message.c_str(), L"Сповіщення встановлено", MB_ICONINFORMATION);
		}
	}

	void EventsDialog::CheckAndShowAlarms(HWND hDlg)
	{
		eventsManager->CheckForAlarms();
		const AlarmNotification* alarms = eventsManager->GetActiveAlarms();
		size_t alarmCount = eventsManager->GetActiveAlarmCount();

		for (size_t i = 0; i < alarmCount; i++)
		{
			const AlarmNotification& alarm = alarms[i];

			if (ShouldShowNotification(alarm))
			{
				ShowNotification(hDlg, alarm);
			}
		}
	}

	bool EventsDialog::ShouldShowNotification(const AlarmNotification& alarm)
	{
		return true;
	}

	void EventsDialog::ShowNotification(HWND hDlg, const AlarmNotification& alarm)
	{
		wchar_t message[512];
		wchar_t title[256];
		wchar_t eventTime[64];  // Variable for "Час події" part of the message

		// Format the time part once to avoid duplication
		swprintf_s(eventTime, L"Час події: %02d:%02d",
			alarm.event->targetDateTime.wHour,
			alarm.event->targetDateTime.wMinute);

		switch (alarm.state)
		{
		case AlarmState::OneHourBefore:
			swprintf_s(title, L"Нагадування (за 1 годину)");
			swprintf_s(message, L"Подія '%s' почнеться протягом 1 години\n%s",
				alarm.event->name.c_str(), eventTime);
			break;

		case AlarmState::ThirtyMinutesBefore:
			swprintf_s(title, L"Нагадування (за 30 хвилин)");
			swprintf_s(message, L"Подія '%s' почнеться протягом 30 хвилин\n%s",
				alarm.event->name.c_str(), eventTime);
			break;

		case AlarmState::FifteenMinutesBefore:
			swprintf_s(title, L"Нагадування (за 15 хвилин)");
			swprintf_s(message, L"Подія '%s' почнеться протягом 15 хвилин\n%s",
				alarm.event->name.c_str(), eventTime);
			break;

		case AlarmState::ThreeMinutesBefore:
			swprintf_s(title, L"Нагадування (за 3 хвилини)");
			swprintf_s(message, L"Подія '%s' ось ось розпочнеться!\n%s",
				alarm.event->name.c_str(), eventTime);
			break;
		}

		MessageBeep(MB_ICONEXCLAMATION);
		MessageBoxW(hDlg, message, title, MB_ICONEXCLAMATION | MB_OK);
	}

#pragma endregion


#pragma region Help Dialog 

	void EventsDialog::ShowInstructions(HWND hDlg)
	{
		DialogBox(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_EVENTSMANAGER_HELP_DIALOG),hDlg, InstructionsProc);
	}

	INT_PTR CALLBACK EventsDialog::InstructionsProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_INITDIALOG:
		{
			std::wstring helpText = L"Інструкція з використання менеджера подій\n\n"

				L"1. Основні функції:\n"
				L"   - Перегляд списку всіх подій\n"
				L"   - Автоматичне оновлення статусу подій (минулі/майбутні)\n"
				L"   - Сортування подій за різними критеріями\n"
				L"   - Налаштування інтервалу автоматичного оновлення\n\n"

				L"2. Керування подіями:\n"
				L"   - Створення нової події:\n"
				L"     * Виберіть дату в календарі\n"
				L"     * Встановіть час\n"
				L"     * Натисніть 'Створити подію'\n"
				L"     * Заповніть деталі події у діалоговому вікні\n\n"
				L"   - Редагування події:\n"
				L"     * Виберіть подію зі списку\n"
				L"     * Натисніть 'Редагувати'\n"
				L"     * Змініть необхідні параметри\n\n"
				L"   - Видалення події:\n"
				L"     * Виберіть подію зі списку\n"
				L"     * Натисніть 'Видалити подію'\n"
				L"     * Підтвердіть видалення\n\n"
				L"   - Очищення минулих подій:\n"
				L"     * Натисніть 'Очистити минулі'\n"
				L"     * Підтвердіть видалення\n\n"

				L"3. Сортування подій:\n"
				L"   - Виберіть критерій сортування з випадаючого списку:\n"
				L"     * За назвою\n"
				L"     * За датою події\n"
				L"     * За важливістю\n"
				L"     * За датою створення\n\n"

				L"4. Налаштування сповіщень:\n"
				L"   - Система автоматично показує сповіщення:\n"
				L"     * За 1 годину до події\n"
				L"     * За 30 хвилин до події\n"
				L"     * За 15 хвилин до події\n"
				L"     * За 3 хвилини до події\n"
				L"     ! Якщо автоматичне оновлення подій вимкнено, сповіщення в режимі 'Не турбувати'\n\n"

				L"5. Робота з файлами:\n"
				L"   - Завантаження подій з файлу:\n"
				L"     * Виберіть 'Файл → Завантажити'\n"
				L"     * Оберіть файл з подіями\n\n"
				L"   - Збереження подій у файл:\n"
				L"     * Виберіть 'Файл → Зберегти' для збереження в поточний файл\n"
				L"     * Або 'Файл → Зберегти як...' для вибору нового файлу\n\n"

				L"6. Особливості:\n"
				L"   - Підтримка повторюваних подій\n"
				L"   - Встановлення рівня важливості (1-5, відображається у вигляді зірок)\n"
				L"   - Детальний перегляд інформації про подію\n"
				L"   - Підсвічування статусу події (активна/минула/повторювана)\n"
				L"   - Попередження при спробі створити подію в минулому\n"
				L"   - Можливість змінити частоту оновлення списку подій,\n"
				L"     або вимкнути їх взагалі (кнопка 'Оновити' для ручного оновлення)\n\n";

			SetDlgItemText(hDlg, IDC_EVENTSMANAGER_HELP_TEXT, helpText.c_str());
			return TRUE;
		}
		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
		}
		return FALSE;
	}

#pragma endregion

}