#include "EventInputDialog.h"
#include <commctrl.h>
#include <strsafe.h>
#include "resource.h"

#pragma comment(lib, "comctl32.lib")

int EventInputDialog::currentTheme = 0;
HFONT EventInputDialog::hFont = nullptr;
Event* EventInputDialog::pEvent = nullptr;

bool EventInputDialog::Show(HWND parent, Event& event, int theme)
{
	currentTheme = theme;
	pEvent = &event;

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
			if (GetInput(hDlg, *pEvent))
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

// Remove the CheckBoxSubclassProc completely and update InitializeControls:

void EventInputDialog::InitializeControls(HWND hDlg, Event& event)
{
	hFont = AppStyles::GetDefaultButtonFont();

	// Initialize owner-draw buttons
	UIHelpers::InitOwnerDrawButton(hDlg, IDOK, hFont);
	UIHelpers::InitOwnerDrawButton(hDlg, IDCANCEL, hFont);

	// Set time picker format to show hours and minutes (updated style)
	HWND hTimePicker = GetDlgItem(hDlg, IDC_EVENT_TIME_PICKER);
	DateTime_SetFormat(hTimePicker, L"HH':'mm");

	// Remove the window styles that prevent minute selection
	LONG_PTR style = GetWindowLongPtr(hTimePicker, GWL_STYLE);
	style &= ~DTS_UPDOWN;  // Remove the up-down control style
	style |= DTS_TIMEFORMAT; // Ensure time format is enabled
	SetWindowLongPtr(hTimePicker, GWL_STYLE, style);

	// Set default time to now + 1 hour
	SYSTEMTIME st;
	GetLocalTime(&st);
	st.wHour = (st.wHour + 1) % 24;
	st.wMinute = 0;
	DateTime_SetSystemtime(hTimePicker, GDT_VALID, &st);

	// Initialize importance spinner (1-5)
	HWND hSpin = GetDlgItem(hDlg, IDC_EVENT_IMPORTANCE_SPIN);
	SendMessage(hSpin, UDM_SETRANGE32, 1, 5);
	SendMessage(hSpin, UDM_SETPOS32, 0, 3); // Default to 3 (medium importance)

	// Set the buddy control text
	SetDlgItemInt(hDlg, IDC_EVENT_IMPORTANCE_EDIT, 3, FALSE);

	// Initialize recurrence spinner (1-365)
	hSpin = GetDlgItem(hDlg, IDC_EVENT_RECURRENCE_SPIN);
	SendMessage(hSpin, UDM_SETRANGE32, 1, 365);
	SendMessage(hSpin, UDM_SETPOS32, 0, 1); // Default to 1 day
	SetDlgItemInt(hDlg, IDC_EVENT_RECURRENCE_EDIT, 1, FALSE);

	// Disable recurrence controls initially
	EnableWindow(GetDlgItem(hDlg, IDC_EVENT_RECURRENCE_EDIT), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_EVENT_RECURRENCE_SPIN), FALSE);

	// Set alarm checkbox to default on
	CheckDlgButton(hDlg, IDC_EVENT_ALARM_CHECK, BST_CHECKED);

	// No need for subclassing checkboxes - use standard behavior
	// Remove the SetWindowSubclass calls

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

	// Get time (hours and minutes)
	SYSTEMTIME time;
	if (DateTime_GetSystemtime(GetDlgItem(hDlg, IDC_EVENT_TIME_PICKER), &time) != GDT_VALID)
	{
		MessageBoxW(hDlg, L"Будь ласка, введіть коректний час", L"Попередження", MB_ICONWARNING);
		return false;
	}
	event.deadline.wHour = time.wHour;
	event.deadline.wMinute = time.wMinute;

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
	GetLocalTime(&event.creationDate);

	return true;
}