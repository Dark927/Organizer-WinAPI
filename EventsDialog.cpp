#define UNICODE
#define _UNICODE

#include "EventsDialog.h"
#include <algorithm>
#include <iomanip>
#include <sstream>
#include "resource.h"
#include "EventInputDialog.h"
#include <richedit.h>
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")

int EventsDialog::currentTheme = 0;
HFONT EventsDialog::hFont = nullptr;
HWND EventsDialog::hCurrentDialog = nullptr;
bool EventsDialog::autoUpdateEnabled = true;
int EventsDialog::updateIntervalMinutes = 1;
static HBRUSH hEditBackgroundBrush = NULL;

EventsManager* eventsManager = nullptr;

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
		currentTheme = (int)lParam;
		hCurrentDialog = hDlg;
		InitializeControls(hDlg);
		LoadEventsToList(hDlg);

		// Set timer for auto-update if enabled
		if (autoUpdateEnabled)
		{
			SetTimer(hDlg, 1, updateIntervalMinutes * 60 * 1000, nullptr);
		}
		return TRUE;

	case WM_TIMER:
		if (wParam == 1)
		{
			eventsManager->UpdateEventsStatus();
			eventsManager->ProcessRecurringEvents();
			LoadEventsToList(hDlg);
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
		case ID_CREATE_EVENT_BTN:
		{
			Event newEvent;

			// Get selected date from calendar
			SYSTEMTIME selectedDate;
			MonthCal_GetCurSel(GetDlgItem(hDlg, IDC_EVENTS_CALENDAR), &selectedDate);

			// Set the initial deadline date from calendar selection
			newEvent.deadline = selectedDate;

			if (EventInputDialog::Show(hDlg, newEvent, currentTheme))
			{
				eventsManager->AddEvent(newEvent);
				LoadEventsToList(hDlg);
			}
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

		case IDC_EVENT_ALARM_BTN:
		{
			SetAlarmForSelectedEvent(hDlg);
			return TRUE;
		}

		case IDC_EVENT_STATS_BTN:
		{
			ShowEventStats(hDlg);
			return TRUE;
		}

		case IDC_EVENT_UPDATE_BTN:
		{
			eventsManager->UpdateEventsStatus();
			eventsManager->ProcessRecurringEvents();
			LoadEventsToList(hDlg);
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

		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		break;


	case WM_CLOSE:
		KillTimer(hDlg, 1);
		EndDialog(hDlg, IDCANCEL);
		return TRUE;

	case WM_DESTROY:
		if (hEditBackgroundBrush)
		{
			DeleteObject(hEditBackgroundBrush);
			hEditBackgroundBrush = NULL;
		}

		delete eventsManager;
		eventsManager = nullptr;
		KillTimer(hDlg, 1);
		if (hFont) DeleteObject(hFont);
		hCurrentDialog = nullptr;
		break;
	}
	return FALSE;
}

void EventsDialog::InitializeControls(HWND hDlg)
{
	hFont = AppStyles::GetDefaultButtonFont();

	HWND hRichEdit = GetDlgItem(hDlg, IDC_EVENT_DETAILS_RICHEDIT);

	// Set background color
	SendMessage(hRichEdit, EM_SETBKGNDCOLOR, 0,
		AppStyles::GetThemeColors(currentTheme).windowBg);

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
	UIHelpers::InitOwnerDrawButton(hDlg, IDC_EVENT_ALARM_BTN, hFont);
	UIHelpers::InitOwnerDrawButton(hDlg, IDC_EVENT_STATS_BTN, hFont);
	UIHelpers::InitOwnerDrawButton(hDlg, IDC_EVENT_UPDATE_BTN, hFont);
	UIHelpers::InitOwnerDrawButton(hDlg, IDC_EVENT_CLEAR_PAST_BTN, hFont);
	UIHelpers::InitOwnerDrawButton(hDlg, IDCANCEL, hFont);

	// Initialize sort combo
	HWND hSortCombo = GetDlgItem(hDlg, IDC_EVENT_SORT_COMBO);
	SendMessage(hSortCombo, CB_ADDSTRING, 0, (LPARAM)L"За назвою");
	SendMessage(hSortCombo, CB_ADDSTRING, 0, (LPARAM)L"За датою");
	SendMessage(hSortCombo, CB_ADDSTRING, 0, (LPARAM)L"За важливістю");
	SendMessage(hSortCombo, CB_ADDSTRING, 0, (LPARAM)L"За датою створення");
	SendMessage(hSortCombo, CB_SETCURSEL, 0, 0);

	UIHelpers::ApplyTheme(hDlg, currentTheme);
}

void EventsDialog::LoadEventsToList(HWND hDlg)
{
	HWND hList = GetDlgItem(hDlg, IDC_EVENTS_LIST);
	SendMessage(hList, LB_RESETCONTENT, 0, 0);

	const auto& allEvents = eventsManager->GetAllEvents();
	for (const auto& event : allEvents)
	{
		// Format date/time
		wchar_t buffer[256];
		swprintf_s(buffer, L"[%02d/%02d/%04d %02d:%02d] %s",
			event.deadline.wDay, event.deadline.wMonth, event.deadline.wYear,
			event.deadline.wHour, event.deadline.wMinute,
			event.name.c_str());

		// Add to list
		SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)buffer);
	}

	// Select first item if available
	if (SendMessage(hList, LB_GETCOUNT, 0, 0) > 0)
	{
		SendMessage(hList, LB_SETCURSEL, 0, 0);
		UpdateEventDetails(hDlg, 0);
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
		const auto& allEvents = eventsManager->GetAllEvents();
		if (selectedIndex >= 0 && selectedIndex < (int)allEvents.size())
		{
			const Event& event = allEvents[selectedIndex];

			// Format your text as before
			std::wstring text = FormatEventDetails(event);

			// Set the text
			SetWindowTextW(hRichEdit, text.c_str());
		}
	}

	SendMessage(hRichEdit, WM_SETREDRAW, TRUE, 0);
	InvalidateRect(hRichEdit, NULL, TRUE);
	UpdateWindow(hRichEdit);

	// Scroll to top
	SendMessage(hRichEdit, EM_SETSEL, 0, 0);
	SendMessage(hRichEdit, EM_SCROLLCARET, 0, 0);
}

std::wstring EventsDialog::FormatEventDetails(const Event& event)
{
	std::wstringstream details;
	details << L"Назва: " << event.name << L"\r\n\r\n";
	details << L"Опис:\r\n" << event.description << L"\r\n\r\n";
	details << L"Нотатки:\r\n" << event.notes << L"\r\n\r\n";
	details << L"Дата/час: "
		<< event.deadline.wDay << L"."
		<< event.deadline.wMonth << L"."
		<< event.deadline.wYear << L" "
		<< event.deadline.wHour << L":"
		<< std::setfill(L'0') << std::setw(2) << event.deadline.wMinute << L"\r\n\r\n";

	int filledStars = event.importance;
	int emptyStars = 5 - filledStars;
	std::wstring stars = L"";

	// Add filled stars
	for (int i = 0; i < filledStars; ++i)
	{
		stars += L"★";
	}

	// Add empty stars
	for (int i = 0; i < emptyStars; ++i)
	{
		stars += L"☆";
	}

	details << L"Важливість: " << stars << L"\r\n\r\n";
	details << L"Стан: " << (event.isCompleted ? L"Виконано" :
		(event.isPastDue ? L"Протерміновано" : L"Активна")) << L"\r\n\r\n";
	details << L"Створено: "
		<< event.creationDate.wDay << L"."
		<< event.creationDate.wMonth << L"."
		<< event.creationDate.wYear << L"\r\n\r\n";
	details << L"Повторювана: " << (event.isRecurring ? L"Так" : L"Ні") << L"\r\n";
	if (event.isRecurring)
	{
		details << L"Інтервал: кожні " << event.recurrenceInterval << L" днів\r\n";
	}
	details << L"Сповіщення: " << (event.hasAlarm ? L"Так" : L"Ні");

	return details.str();
}


void EventsDialog::SetAlarmForSelectedEvent(HWND hDlg)
{
	int selected = SendDlgItemMessage(hDlg, IDC_EVENTS_LIST, LB_GETCURSEL, 0, 0);
	if (selected == LB_ERR)
	{
		MessageBoxW(hDlg, L"Будь ласка, виберіть подію для налаштування сповіщення",
			L"Попередження", MB_ICONWARNING);
		return;
	}

	const auto& allEvents = eventsManager->GetAllEvents();
	if (selected >= 0 && selected < (int)allEvents.size())
	{
		const Event& event = allEvents[selected];
		std::wstring message = L"Сповіщення для події: " + event.name;
		MessageBoxW(hDlg, message.c_str(), L"Сповіщення встановлено", MB_ICONINFORMATION);
	}
}

void EventsDialog::ShowEventStats(HWND hParent)
{
	auto weeklyStats = eventsManager->GetWeeklyStats();
	auto monthlyStats = eventsManager->GetMonthlyStats();

	std::wstringstream stats;
	stats << L"Статистика за тиждень:\r\n";
	stats << L"Всього подій: " << weeklyStats.total << L"\r\n";
	stats << L"Виконано: " << weeklyStats.completed << L"\r\n";
	stats << L"Відсоток виконання: " << weeklyStats.completionRate << L"%\r\n\r\n";

	stats << L"Статистика за місяць:\r\n";
	stats << L"Всього подій: " << monthlyStats.total << L"\r\n";
	stats << L"Виконано: " << monthlyStats.completed << L"\r\n";
	stats << L"Відсоток виконання: " << monthlyStats.completionRate << L"%";

	MessageBoxW(hParent, stats.str().c_str(), L"Статистика подій", MB_ICONINFORMATION);
}