#include "ContactBookDialog.h"
#include "ContactInputDialog.h"
#include <commctrl.h>
#include "resource.h"

HFONT ContactBookDialog::hFont = nullptr;
int ContactBookDialog::currentTheme = 0;
ContactBook* ContactBookDialog::contactBook = nullptr;

void ContactBookDialog::Show(HWND parent, HINSTANCE hInst, int theme)
{
	currentTheme = theme;
	contactBook = new ContactBook();
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CONTACT_BOOK_DIALOG), parent,
		DialogProc, (LPARAM)theme);
}

INT_PTR CALLBACK ContactBookDialog::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG: {
		currentTheme = (int)lParam;
		hFont = AppStyles::GetDefaultButtonFont();
		InitializeControls(hDlg);
		UIHelpers::ApplyTheme(hDlg, currentTheme, false);
		InitializeComboboxes(hDlg);
		return TRUE;
	}

	case WM_DRAWITEM:
		if (UIHelpers::DrawThemedButton((LPDRAWITEMSTRUCT)lParam, currentTheme))
		{
			return TRUE;
		}
		break;

	case WM_CTLCOLORDLG:
		return (LRESULT)CreateSolidBrush(AppStyles::GetThemeColors(currentTheme).windowBg);

	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLORBTN:
	{
		HDC hdc = (HDC)wParam;
		SetTextColor(hdc, AppStyles::GetThemeColors(currentTheme).titleText);
		SetBkMode(hdc, TRANSPARENT);
		return (LRESULT)CreateSolidBrush(AppStyles::GetThemeColors(currentTheme).windowBg);
	}

	case WM_COMMAND: {
		if (MenuHelpers::HandleMenuCommand(hDlg, wParam, lParam))
		{
			switch (LOWORD(wParam))
			{
			case IDM_HELP_INSTRUCTIONS:
				DialogBox(GetModuleHandle(NULL),
					MAKEINTRESOURCE(IDD_INSTRUCTIONS_DIALOG),
					hDlg, InstructionsProc);
				break;
			case IDM_FILE_EXIT:
				SendMessage(hDlg, WM_CLOSE, 0, 0);
				break;
			}
			return TRUE;
		}

		switch (LOWORD(wParam))
		{
		case IDC_COPY_BUTTON: {
			// Copy selected column implementation would go here
			break;
		}
		case IDC_SORT_BUTTON: {
			// Sort by selected column implementation would go here
			break;
		}
		case IDC_RESET_BUTTON: {
			// Reset filters/sorting implementation would go here
			break;
		}
		case IDC_CONTACTS_ADD_CONTACT: {
			Contact newContact;
			if (ContactInputDialog::Show(hDlg, newContact, currentTheme))
			{
				contactBook->AddContact(newContact);
				LoadContacts(GetDlgItem(hDlg, IDC_CONTACTS_CONTACT_LIST));
			}
			break;
		}

		case IDC_CONTACTS_DELETE_CONTACT: {
			HWND hList = GetDlgItem(hDlg, IDC_CONTACTS_CONTACT_LIST);
			int selected = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
			if (selected != -1)
			{
				contactBook->DeleteContact(selected);
				LoadContacts(hList);
			}
			break;
		}
		case IDC_CONTACTS_COPY_PHONE:
			// Implement copy phone
			break;

		case IDC_CONTACTS_COPY_NAME:
			// Implement copy name
			break;

		case IDC_CONTACTS_FILTER:
			contactBook->FilterContacts("work");
			break;

		case IDC_CONTACTS_SORT_DATE:
			contactBook->SortContacts(0);
			LoadContacts(GetDlgItem(hDlg, IDC_CONTACTS_CONTACT_LIST));
			break;

		case IDC_CONTACTS_SORT_NAME:
			contactBook->SortContacts(1);
			LoadContacts(GetDlgItem(hDlg, IDC_CONTACTS_CONTACT_LIST));
			break;

		case IDC_CONTACTS_SORT_COUNTRY:
			contactBook->SortContacts(2);
			LoadContacts(GetDlgItem(hDlg, IDC_CONTACTS_CONTACT_LIST));
			break;

		case ID_CONTACTS_CLOSE:
			EndDialog(hDlg, ID_CONTACTS_CLOSE);
			return TRUE;
		}
		break;
	}

	case WM_CLOSE:
		delete contactBook;
		contactBook = nullptr;
		EndDialog(hDlg, ID_CONTACTS_CLOSE);
		return TRUE;

	case WM_DESTROY:
		if (hFont) DeleteObject(hFont);
		break;
	}
	return FALSE;
}
INT_PTR CALLBACK ContactBookDialog::InstructionsProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;
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

void ContactBookDialog::InitializeComboboxes(HWND hDlg)
{
	HWND hCopyCombo = GetDlgItem(hDlg, IDC_COPY_COLUMN_COMBO);
	HWND hSortCombo = GetDlgItem(hDlg, IDC_SORT_COLUMN_COMBO);

	const char* columns[] = {
		"Name", "Phone", "Country", "Address", "Tags", "Date Added"
	};

	for (const char* col : columns)
	{
		SendMessageA(hCopyCombo, CB_ADDSTRING, 0, (LPARAM)col);
		SendMessageA(hSortCombo, CB_ADDSTRING, 0, (LPARAM)col);
	}

	SendMessage(hCopyCombo, CB_SETCURSEL, 0, 0);
	SendMessage(hSortCombo, CB_SETCURSEL, 0, 0);
}

void ContactBookDialog::InitializeControls(HWND hDlg)
{
	// Initialize owner-draw buttons
	UIHelpers::InitOwnerDrawButton(hDlg, IDC_CONTACTS_ADD_CONTACT, hFont);
	UIHelpers::InitOwnerDrawButton(hDlg, IDC_CONTACTS_DELETE_CONTACT, hFont);
	UIHelpers::InitOwnerDrawButton(hDlg, IDC_COPY_BUTTON, hFont);
	UIHelpers::InitOwnerDrawButton(hDlg, IDC_SORT_BUTTON, hFont);
	UIHelpers::InitOwnerDrawButton(hDlg, IDC_RESET_BUTTON, hFont);
	UIHelpers::InitOwnerDrawButton(hDlg, ID_CONTACTS_CLOSE, hFont);

	// Initialize ListView
	HWND hList = GetDlgItem(hDlg, IDC_CONTACTS_CONTACT_LIST);
	ListView_SetExtendedListViewStyle(hList,
		LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES | LVS_EX_GRIDLINES);

	LVCOLUMN lvc = { 0 };
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

	// Favorite column
	lvc.iSubItem = 0;
	lvc.pszText = const_cast<LPSTR>("Обраний");
	lvc.cx = 60;
	lvc.fmt = LVCFMT_CENTER;
	ListView_InsertColumn(hList, 0, &lvc);

	// Name column
	lvc.iSubItem = 1;
	lvc.pszText = const_cast<LPSTR>("Ім'я");
	lvc.cx = 150;
	lvc.fmt = LVCFMT_LEFT;
	ListView_InsertColumn(hList, 1, &lvc);

	// Phone column
	lvc.iSubItem = 2;
	lvc.pszText = const_cast<LPSTR>("Номер телефону");
	lvc.cx = 120;
	ListView_InsertColumn(hList, 2, &lvc);

	// Country column
	lvc.iSubItem = 3;
	lvc.pszText = const_cast<LPSTR>("Країна");
	lvc.cx = 100;
	ListView_InsertColumn(hList, 3, &lvc);

	// Address column
	lvc.iSubItem = 4;
	lvc.pszText = const_cast<LPSTR>("Адреса");
	lvc.cx = 150;
	ListView_InsertColumn(hList, 4, &lvc);

	// Date added column
	lvc.iSubItem = 5;
	lvc.pszText = const_cast<LPSTR>("Дата додавання");
	lvc.cx = 120;
	ListView_InsertColumn(hList, 5, &lvc);

	// Tags column
	lvc.iSubItem = 6;
	lvc.pszText = const_cast<LPSTR>("Теги");
	lvc.cx = 100;
	ListView_InsertColumn(hList, 6, &lvc);

	LoadContacts(hList);
}

void ContactBookDialog::LoadContacts(HWND hList)
{
	ListView_DeleteAllItems(hList);

	const auto& contacts = contactBook->GetContacts();
	for (size_t i = 0; i < contacts.size(); i++)
	{
		const Contact& c = contacts[i];

		LVITEM lvi = { 0 };
		lvi.mask = LVIF_TEXT | LVIF_PARAM;
		lvi.iItem = (int)i;
		lvi.lParam = (LPARAM)i;

		int itemIdx = ListView_InsertItem(hList, &lvi);

		// Set checkbox state for favorite
		ListView_SetCheckState(hList, itemIdx, c.isFavorite);

		// Name (column 1)
		ListView_SetItemText(hList, itemIdx, 1, const_cast<LPSTR>(c.name.c_str()));

		// Phone (column 2)
		ListView_SetItemText(hList, itemIdx, 2, const_cast<LPSTR>(c.phone.c_str()));

		// Country (column 3)
		ListView_SetItemText(hList, itemIdx, 3, const_cast<LPSTR>(c.country.c_str()));

		// Address (column 4)
		ListView_SetItemText(hList, itemIdx, 4, const_cast<LPSTR>(c.address.c_str()));

		// Date added (column 5)
		char dateStr[32];
		sprintf_s(dateStr, "%04d-%02d-%02d",
			c.addedDate.wYear, c.addedDate.wMonth, c.addedDate.wDay);
		ListView_SetItemText(hList, itemIdx, 5, dateStr);

		// Tags (column 6)
		ListView_SetItemText(hList, itemIdx, 6, const_cast<LPSTR>(c.tags.c_str()));
	}

	UpdateWindow(hList);
}