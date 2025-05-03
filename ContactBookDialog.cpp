#define UNICODE
#include "ContactBookDialog.h"
#include "ContactInputDialog.h"
#include <commctrl.h>
#include <commdlg.h>  // Required for the file dialog
#include "resource.h"
#pragma comment(lib, "comctl32.lib")

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

		RegisterHotkeys(hDlg);

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

	case WM_HOTKEY: {
		// Forward hotkey messages as if they were menu commands
		SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(wParam, 0), 0);
		return TRUE;
	}

	case WM_COMMAND: {
		if (MenuHelpers::HandleMenuCommand(hDlg, wParam, lParam))
		{
			if (HandleConcreteMenuCommand(wParam, hDlg) == TRUE) return TRUE;
			break;
		}

		switch (LOWORD(wParam))
		{

		case IDC_COPY_BUTTON:
		{
			CopyInfoFromSelectedContact(hDlg);
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
				LoadContactsToListView(GetDlgItem(hDlg, IDC_CONTACTS_CONTACT_LIST));
			}
			break;
		}

		case IDC_CONTACTS_DELETE_CONTACT: {
			HWND hList = GetDlgItem(hDlg, IDC_CONTACTS_CONTACT_LIST);
			int selected = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
			if (selected != -1)
			{
				contactBook->DeleteContact(selected);
				LoadContactsToListView(hList);
			}
			break;
		}

		case IDC_CONTACTS_FILTER:
			contactBook->FilterContacts(L"work");
			break;

		case ID_CONTACTS_CLOSE:
			EndDialog(hDlg, ID_CONTACTS_CLOSE);
			return TRUE;
		}
		break;
	}

	case WM_TIMER:
		if (wParam == 1)
		{
			KillTimer(hDlg, 1);
			SetDlgItemText(hDlg, IDC_COPY_BUTTON, L"Копіювати");
		}
		break;

	case WM_KEYDOWN:
		if (wParam == VK_F1)
		{
			PostMessage(hDlg, WM_COMMAND, IDM_HELP_INSTRUCTIONS, 0);
			return 0;
		}
		break;

	case WM_CLOSE:
		delete contactBook;
		contactBook = nullptr;
		EndDialog(hDlg, ID_CONTACTS_CLOSE);
		return TRUE;

	case WM_DESTROY:
		//UnregisterHotkeys(hDlg);
		if (hFont) DeleteObject(hFont);
		break;
	}
	return FALSE;
}

void ContactBookDialog::UnregisterHotkeys(HWND hDlg)
{
	UnregisterHotKey(hDlg, IDM_HELP_INSTRUCTIONS);
	UnregisterHotKey(hDlg, IDM_FILE_LOAD);
	UnregisterHotKey(hDlg, IDM_FILE_SAVE);
	UnregisterHotKey(hDlg, IDM_FILE_SAVEAS);
	UnregisterHotKey(hDlg, IDM_FILE_EXIT);
}

void ContactBookDialog::RegisterHotkeys(HWND hDlg)
{
	RegisterHotKey(hDlg, IDM_HELP_INSTRUCTIONS, 0, VK_F1);
	RegisterHotKey(hDlg, IDM_FILE_LOAD, MOD_CONTROL, 'O');
	RegisterHotKey(hDlg, IDM_FILE_SAVE, MOD_CONTROL, 'S');
	RegisterHotKey(hDlg, IDM_FILE_SAVEAS, MOD_CONTROL | MOD_SHIFT, 'S');
	RegisterHotKey(hDlg, IDM_FILE_EXIT, MOD_ALT, VK_F4);
}

INT_PTR ContactBookDialog::HandleConcreteMenuCommand(WPARAM& wParam, HWND& hDlg)
{
	switch (LOWORD(wParam))
	{
	case IDM_HELP_INSTRUCTIONS:
		// Handle the "Instructions" menu item (F1)
		DialogBox(GetModuleHandle(NULL),
			MAKEINTRESOURCE(IDD_INSTRUCTIONS_DIALOG),
			hDlg, InstructionsProc);
		break;


	case IDM_FILE_LOAD:
	{
		LoadContactBookFromFile(hDlg);
	}
	break;


	case IDM_FILE_SAVE: 
	{
		if (!contactBook->SaveToCurrentFile())
		{
			// If saving to current file fails, show warning and prompt for new file
			int result = MessageBox(hDlg,
				L"Не вдалося зберегти контакти в поточний файл. Хочете вибрати інший файл для збереження?",
				L"Помилка збереження",
				MB_YESNO | MB_ICONWARNING);

			if (result == IDYES)
			{
				SaveContactBookAs(hDlg);
			}
		}
		else
		{
			MessageBox(hDlg,
				L"Контакти успішно збережено",
				L"Успішне збереження",
				MB_OK | MB_ICONINFORMATION);
		}
		break;
	}

	case IDM_FILE_SAVEAS:
		SaveContactBookAs(hDlg);
		break;

	case IDM_FILE_EXIT:
		SendMessage(hDlg, WM_CLOSE, 0, 0);
		break;

	default:
		return FALSE;
	}

	return TRUE; // If handled successfully
}

void ContactBookDialog::SaveContactBookAs(HWND hDlg)
{
	// Initialize Save As dialog
	OPENFILENAME ofn = { 0 };
	wchar_t szFile[MAX_PATH] = L"contacts.txt";  // Default filename

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hDlg;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
	ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrDefExt = L"txt";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

	// Show the Save As dialog
	if (GetSaveFileName(&ofn))
	{
		if (contactBook->SaveToFile(szFile))
		{
			MessageBox(hDlg,
				L"Контакти успішно збережено у новий файл",
				L"Успішне збереження",
				MB_OK | MB_ICONINFORMATION);
		}
		else
		{
			MessageBox(hDlg,
				L"Не вдалося зберегти контакти у вказаний файл",
				L"Помилка збереження",
				MB_OK | MB_ICONERROR);
		}
	}
}

void ContactBookDialog::LoadContactBookFromFile(HWND& hDlg)
{
	// Initialize the OPENFILENAME structure
	OPENFILENAME ofn = { 0 };
	wchar_t szFile[MAX_PATH] = L"";  // Buffer to store the file path

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hDlg;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
	ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrDefExt = L"txt";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	// Show the Open dialog
	if (GetOpenFileName(&ofn))
	{
		// Attempt to load the file
		if (contactBook->LoadFromFile(szFile))
		{
			// Refresh the contact list
			LoadContactsToListView(GetDlgItem(hDlg, IDC_CONTACTS_CONTACT_LIST));

			// Show success message in Ukrainian
			MessageBox(hDlg,
				L"Контакти успішно завантажено з файлу",
				L"Успішне завантаження",
				MB_OK | MB_ICONINFORMATION);
		}
		else
		{
			// Show error message in Ukrainian
			MessageBox(hDlg,
				L"Не вдалося завантажити контакти з файлу",
				L"Помилка завантаження",
				MB_OK | MB_ICONERROR);
		}
	}
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

	const wchar_t* columns[] = {
		L"Ім'я", L"Телефон", L"Країна", L"Адрес", L"Дата додавання", L"Теги"
	};

	for (const wchar_t* col : columns)
	{
		SendMessage(hCopyCombo, CB_ADDSTRING, 0, (LPARAM)col);
		SendMessage(hSortCombo, CB_ADDSTRING, 0, (LPARAM)col);
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
		LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	LVCOLUMN lvc = { 0 };
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

	// Favorite column
	lvc.iSubItem = 1;
	lvc.pszText = const_cast<LPWSTR>(L"Обраний");
	lvc.cx = 70;
	lvc.fmt = LVCFMT_CENTER;
	ListView_InsertColumn(hList, 1, &lvc);

	// Name column
	lvc.iSubItem = 1;
	lvc.pszText = const_cast<LPWSTR>(L"Ім'я");
	lvc.cx = 150;
	lvc.fmt = LVCFMT_LEFT;
	ListView_InsertColumn(hList, 1, &lvc);

	// Phone column
	lvc.iSubItem = 2;
	lvc.pszText = const_cast<LPWSTR>(L"Номер телефону");
	lvc.cx = 120;
	ListView_InsertColumn(hList, 2, &lvc);

	// Country column
	lvc.iSubItem = 3;
	lvc.pszText = const_cast<LPWSTR>(L"Країна");
	lvc.cx = 100;
	ListView_InsertColumn(hList, 3, &lvc);

	// Address column
	lvc.iSubItem = 4;
	lvc.pszText = const_cast<LPWSTR>(L"Адреса");
	lvc.cx = 150;
	ListView_InsertColumn(hList, 4, &lvc);

	// Date added column
	lvc.iSubItem = 5;
	lvc.pszText = const_cast<LPWSTR>(L"Дата додавання");
	lvc.cx = 120;
	ListView_InsertColumn(hList, 5, &lvc);

	// Tags column
	lvc.iSubItem = 6;
	lvc.pszText = const_cast<LPWSTR>(L"Теги");
	lvc.cx = 100;
	ListView_InsertColumn(hList, 6, &lvc);

	LoadContactsToListView(hList);
}

void ContactBookDialog::LoadContactsToListView(HWND hList)
{
	// Disable redraw during loading to prevent flickering
	SendMessage(hList, WM_SETREDRAW, FALSE, 0);

	// Clear existing items
	ListView_DeleteAllItems(hList);

	const Contact* contacts = contactBook->GetContacts();
	size_t contactCount = contactBook->GetContactCount();

	// Set the list view to Unicode mode
	ListView_SetUnicodeFormat(hList, TRUE);

	for (size_t i = 0; i < contactCount; i++)
	{
		const Contact& c = contacts[i];

		LVITEM lvi = { 0 };
		lvi.mask = LVIF_TEXT | LVIF_PARAM;
		lvi.iItem = (int)i;
		lvi.lParam = (LPARAM)i;

		// Insert the item
		int itemIdx = ListView_InsertItem(hList, &lvi);

		ListView_SetItemText(hList, itemIdx, 0, const_cast<LPWSTR>((wchar_t*)(c.isFavorite ? "*" : " ")));
		ListView_SetItemText(hList, itemIdx, 1, const_cast<LPWSTR>(c.name.c_str()));
		ListView_SetItemText(hList, itemIdx, 2, const_cast<LPWSTR>(c.phone.c_str()));
		ListView_SetItemText(hList, itemIdx, 3, const_cast<LPWSTR>(c.country.c_str()));
		ListView_SetItemText(hList, itemIdx, 4, const_cast<LPWSTR>(c.address.c_str()));

		// Format date
		wchar_t dateStr[32];
		swprintf_s(dateStr, L"%04d-%02d-%02d",
			c.addedDate.wYear, c.addedDate.wMonth, c.addedDate.wDay);
		ListView_SetItemText(hList, itemIdx, 5, dateStr);

		ListView_SetItemText(hList, itemIdx, 6, const_cast<LPWSTR>(c.tags.c_str()));
	}

	// Re-enable redraw and force a repaint
	SendMessage(hList, WM_SETREDRAW, TRUE, 0);
	RedrawWindow(hList, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
}




void ContactBookDialog::CopyInfoFromSelectedContact(HWND hDlg)
{
	HWND hList = GetDlgItem(hDlg, IDC_CONTACTS_CONTACT_LIST);
	HWND hCombo = GetDlgItem(hDlg, IDC_COPY_COLUMN_COMBO);

	// Get selected contact
	int selectedItem = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
	if (selectedItem == -1)
	{
		MessageBox(hDlg, L"Будь-ласка оберіть контакт для копіювання", L"No Selection", MB_ICONINFORMATION);
		return;
	}

	// Get selected column from combo box
	int selectedColumn = (int)SendMessage(hCombo, CB_GETCURSEL, 0, 0);
	if (selectedColumn == CB_ERR)
	{
		MessageBox(hDlg, L"Будь-ласка оберіть колонку яку необхідно скопіювати", L"No Column Selected", MB_ICONINFORMATION);
		return;
	}

	// Map combo box selection to list view column index
	// Assuming combo box items are in the same order as columns (excluding checkbox column)
	int columnIndex = selectedColumn + 1; // +1 to skip checkbox column

	// Get the text from the selected cell
	wchar_t buffer[256] = { 0 };
	ListView_GetItemText(hList, selectedItem, columnIndex, buffer, ARRAYSIZE(buffer));

	// Check if the target column is empty
	if (wcslen(buffer) == 0)
	{
		MessageBox(hDlg, L"Колонка пуста. Не можна виконати копіювання.", L"Empty Column", MB_ICONWARNING);
		return;
	}

	// Copy to clipboard
	if (OpenClipboard(hDlg))
	{
		EmptyClipboard();

		// Allocate global memory for the text
		size_t bufferSize = (wcslen(buffer) + 1) * sizeof(wchar_t);
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, bufferSize);
		if (hMem)
		{
			wchar_t* pMem = (wchar_t*)GlobalLock(hMem);
			if (pMem)
			{
				wcscpy_s(pMem, bufferSize / sizeof(wchar_t), buffer);
				GlobalUnlock(hMem);

				// Set clipboard data
				SetClipboardData(CF_UNICODETEXT, hMem);
			}
		}
		CloseClipboard();

		// Show feedback to user
		std::wstring message = L"Скопійовано";
		SetDlgItemText(hDlg, IDC_COPY_BUTTON, message.c_str());

		// Reset button text after a short delay
		SetTimer(hDlg, 1, 2000, NULL); // 2 seconds
	}
}
