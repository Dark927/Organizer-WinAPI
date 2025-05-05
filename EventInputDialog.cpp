#include "EventInputDialog.h"
#include <commctrl.h>
#include <strsafe.h>
#include "resource.h"
#include "Utils.h"

#pragma comment(lib, "comctl32.lib")

namespace EventsManagerControl
{
	int EventInputDialog::currentTheme = 0;
	HFONT EventInputDialog::hFont = nullptr;
	Event* EventInputDialog::pEvent = nullptr;
	EventInputDialog::Mode EventInputDialog::currentMode = Mode::Create;

	bool EventInputDialog::Show(HWND parent, Event& event, int theme, EventInputDialog::Mode mode)
	{
		currentTheme = theme;
		pEvent = &event;
		currentMode = mode;

		return DialogBoxParam(GetModuleHandle(NULL),
			MAKEINTRESOURCE(IDD_EVENT_INPUT_DIALOG),
			parent,
			DialogProc,
			(LPARAM)theme) == IDOK;
	}

	INT_PTR CALLBACK EventInputDialog::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_INITDIALOG:
			currentTheme = (int)lParam;
			InitializeControls(hDlg, *pEvent);

			// Set character limits
			SendDlgItemMessage(hDlg, IDC_EVENT_NAME_EDIT, EM_SETLIMITTEXT, MAX_NAME_LENGTH, 0);
			SendDlgItemMessage(hDlg, IDC_EVENT_DESCRIPTION_EDIT, EM_SETLIMITTEXT, MAX_DESC_LENGTH, 0);
			SendDlgItemMessage(hDlg, IDC_EVENT_NOTES_EDIT, EM_SETLIMITTEXT, MAX_NOTES_LENGTH, 0);

			// Set initial character counts
			UpdateCharCount(hDlg, IDC_EVENT_NAME_EDIT, IDC_EVENT_NAME_COUNT, MAX_NAME_LENGTH);
			UpdateCharCount(hDlg, IDC_EVENT_DESCRIPTION_EDIT, IDC_EVENT_DESC_COUNT, MAX_DESC_LENGTH);
			UpdateCharCount(hDlg, IDC_EVENT_NOTES_EDIT, IDC_EVENT_NOTES_COUNT, MAX_NOTES_LENGTH);

			return TRUE;


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
			case IDC_EVENT_NAME_EDIT:
			case IDC_EVENT_DESCRIPTION_EDIT:
			case IDC_EVENT_NOTES_EDIT:
				if (HIWORD(wParam) == EN_CHANGE)
				{
					// Update character count
					int idCount = 0;
					int maxLength = 0;

					switch (LOWORD(wParam))
					{
					case IDC_EVENT_NAME_EDIT:
						idCount = IDC_EVENT_NAME_COUNT;
						maxLength = MAX_NAME_LENGTH;
						break;
					case IDC_EVENT_DESCRIPTION_EDIT:
						idCount = IDC_EVENT_DESC_COUNT;
						maxLength = MAX_DESC_LENGTH;
						break;
					case IDC_EVENT_NOTES_EDIT:
						idCount = IDC_EVENT_NOTES_COUNT;
						maxLength = MAX_NOTES_LENGTH;
						break;
					}

					UpdateCharCount(hDlg, LOWORD(wParam), idCount, maxLength);
				}
				break;

			case IDOK:
				if (Utils::IsDateTimeInPast(pEvent->targetDateTime))
				{
					MessageBoxW(hDlg,
						L"Час події минув під час створення! Будь ласка, перевірте дату та час у вікні менеджера подій.",
						L"Інформація про створення події",
						MB_OK | MB_ICONEXCLAMATION);

					SendMessage(hDlg, WM_CLOSE, 0, 0);
				}
				else if (GetInput(hDlg, *pEvent))
				{
					EndDialog(hDlg, IDOK);
				}
				return TRUE;

			case IDCANCEL:
				EndDialog(hDlg, IDCANCEL);
				return TRUE;

			case IDC_EVENT_RECURRING_CHECK:
				if (HIWORD(wParam) == BN_CLICKED)
				{
					BOOL enabled = IsDlgButtonChecked(hDlg, IDC_EVENT_RECURRING_CHECK);
					EnableWindow(GetDlgItem(hDlg, IDC_EVENT_RECURRENCE_EDIT), enabled);
					EnableWindow(GetDlgItem(hDlg, IDC_EVENT_RECURRENCE_SPIN), enabled);
				}
				return TRUE;
			}
			break;

		case WM_CLOSE:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		return FALSE;
	}

	void EventInputDialog::UpdateCharCount(HWND hDlg, int editId, int countId, int maxLength)
	{
		int length = GetWindowTextLength(GetDlgItem(hDlg, editId));
		wchar_t countText[32];
		swprintf_s(countText, L"%d/%d", length, maxLength);
		SetDlgItemTextW(hDlg, countId, countText);
	}


	void EventInputDialog::InitializeControls(HWND hDlg, Event& event)
	{
		hFont = AppStyles::GetDefaultButtonFont();

		UIHelpers::InitOwnerDrawButton(hDlg, IDOK, hFont);
		UIHelpers::InitOwnerDrawButton(hDlg, IDCANCEL, hFont);

		// Set initial values from the event
		SetDlgItemTextW(hDlg, IDC_EVENT_NAME_EDIT, event.name.c_str());
		SetDlgItemTextW(hDlg, IDC_EVENT_DESCRIPTION_EDIT, event.description.c_str());
		SetDlgItemTextW(hDlg, IDC_EVENT_NOTES_EDIT, event.notes.c_str());

		// Importance
		HWND hSpin = GetDlgItem(hDlg, IDC_EVENT_IMPORTANCE_SPIN);
		HWND hEdit = GetDlgItem(hDlg, IDC_EVENT_IMPORTANCE_EDIT);
		SendMessage(hSpin, UDM_SETBUDDY, (WPARAM)hEdit, 0);
		SendMessage(hSpin, UDM_SETRANGE32, 1, 5);
		SendMessage(hSpin, UDM_SETPOS32, 0, event.importance);
		SetDlgItemInt(hDlg, IDC_EVENT_IMPORTANCE_EDIT, event.importance, FALSE);

		// Alarm
		CheckDlgButton(hDlg, IDC_EVENT_ALARM_CHECK, event.hasAlarm ? BST_CHECKED : BST_UNCHECKED);

		// Recurring
		CheckDlgButton(hDlg, IDC_EVENT_RECURRING_CHECK, event.isRecurring ? BST_CHECKED : BST_UNCHECKED);
		hSpin = GetDlgItem(hDlg, IDC_EVENT_RECURRENCE_SPIN);
		hEdit = GetDlgItem(hDlg, IDC_EVENT_RECURRENCE_EDIT);
		SendMessage(hSpin, UDM_SETBUDDY, (WPARAM)hEdit, 0);
		SendMessage(hSpin, UDM_SETRANGE32, 1, 365);
		SendMessage(hSpin, UDM_SETPOS32, 0, event.recurrenceInterval);
		SetDlgItemInt(hDlg, IDC_EVENT_RECURRENCE_EDIT, event.recurrenceInterval, FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_EVENT_RECURRENCE_EDIT), event.isRecurring);
		EnableWindow(GetDlgItem(hDlg, IDC_EVENT_RECURRENCE_SPIN), event.isRecurring);

		if (currentMode == Mode::Edit)
		{
			SetDlgItemTextW(hDlg, IDOK, L"Зберегти зміни");
		}

		UIHelpers::ApplyTheme(hDlg, currentTheme);
	}

	// Subclass procedure for checkboxes to ensure proper theming
	LRESULT CALLBACK EventInputDialog::CheckBoxSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
	{
		switch (uMsg)
		{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
			// Toggle checkbox state
			CheckDlgButton(GetParent(hWnd), GetDlgCtrlID(hWnd),
				IsDlgButtonChecked(GetParent(hWnd), GetDlgCtrlID(hWnd)) ? BST_UNCHECKED : BST_CHECKED);
			return 0;

		case WM_NCDESTROY:
			RemoveWindowSubclass(hWnd, CheckBoxSubclassProc, uIdSubclass);
			break;
		}
		return DefSubclassProc(hWnd, uMsg, wParam, lParam);
	}

	bool EventInputDialog::GetInput(HWND hDlg, Event& event)
	{
		// Get name
		wchar_t name[MAX_NAME_LENGTH + 1];
		GetDlgItemTextW(hDlg, IDC_EVENT_NAME_EDIT, name, MAX_NAME_LENGTH + 1);
		event.name = name;

		// Validate name
		if (event.name.empty())
		{
			MessageBoxW(hDlg, L"Будь ласка, введіть назву події", L"Попередження", MB_ICONWARNING);
			SetFocus(GetDlgItem(hDlg, IDC_EVENT_NAME_EDIT));
			return false;
		}

		// Get description
		wchar_t desc[MAX_DESC_LENGTH + 1];
		GetDlgItemTextW(hDlg, IDC_EVENT_DESCRIPTION_EDIT, desc, MAX_DESC_LENGTH + 1);
		event.description = desc;

		// Get notes
		wchar_t notes[MAX_NOTES_LENGTH + 1];
		GetDlgItemTextW(hDlg, IDC_EVENT_NOTES_EDIT, notes, MAX_NOTES_LENGTH + 1);
		event.notes = notes;

		// Get importance (1-5)
		BOOL success;
		UINT importance = GetDlgItemInt(hDlg, IDC_EVENT_IMPORTANCE_EDIT, &success, FALSE);
		if (!success || importance < 1 || importance > 5)
		{
			MessageBoxW(hDlg, L"Важливість повинна бути від 1 до 5", L"Попередження", MB_ICONWARNING);
			SetFocus(GetDlgItem(hDlg, IDC_EVENT_IMPORTANCE_EDIT));
			return false;
		}
		event.importance = importance;

		// Get alarm setting
		event.hasAlarm = (IsDlgButtonChecked(hDlg, IDC_EVENT_ALARM_CHECK) == BST_CHECKED);

		// Get recurring setting
		event.isRecurring = (IsDlgButtonChecked(hDlg, IDC_EVENT_RECURRING_CHECK) == BST_CHECKED);
		if (event.isRecurring)
		{
			event.recurrenceInterval = GetDlgItemInt(hDlg, IDC_EVENT_RECURRENCE_EDIT, &success, FALSE);
			if (!success || event.recurrenceInterval < 1 || event.recurrenceInterval > 365)
			{
				MessageBoxW(hDlg, L"Інтервал повинен бути від 1 до 365 днів", L"Попередження", MB_ICONWARNING);
				SetFocus(GetDlgItem(hDlg, IDC_EVENT_RECURRENCE_EDIT));
				return false;
			}
		}

		// Set creation date
		if (currentMode == Mode::Create)
		{
			GetLocalTime(&event.creationDate);
		}

		return true;
	}
}