#define UNICODE

#include "ContactBookDialog.h"
#include "ContactInputDialog.h"
#include <commctrl.h>
#include <commdlg.h> 
#include "resource.h"

#include "Utils.h"
#include "HotkeysHandler.h"
#pragma comment(lib, "comctl32.lib")

namespace ContactBookControl
{
	HFONT ContactBookDialog::hFont = nullptr;
	int ContactBookDialog::currentTheme = 0;
	ContactBook* ContactBookDialog::contactBook = nullptr;
	std::wstring lastFile;
	size_t filteredContactsCount = 0;
	Contact* filteredContacts = nullptr;

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
		case WM_INITDIALOG:
		{
			InitializeContactBookDialog(lParam, hDlg);
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

		case WM_HOTKEY:
		{
			SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(wParam, 0), 0);
			return TRUE;
		}

		case WM_COMMAND:
		{
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

			case IDC_SORT_BUTTON:
			{
				HWND hSortCombo = GetDlgItem(hDlg, IDC_SORT_COLUMN_COMBO);
				int selected = (int)SendMessage(hSortCombo, CB_GETCURSEL, 0, 0);

				if (selected != CB_ERR)
				{
					// Map combobox selection to SortType
					ContactBook::SortType sortType = GetSortTypeFromColumnIndex(selected);

					// Perform the sorting
					contactBook->SortContacts(sortType);

					// Refresh the list view
					LoadContactsToListView(GetDlgItem(hDlg, IDC_CONTACTS_CONTACT_LIST));
				}
				break;
			}

			case IDC_RESET_BUTTON:
			{
				contactBook->DiscardChanges();
				DisposeFilteredContacts();
				ResetUI(hDlg);
				LoadContactsToListView(GetDlgItem(hDlg, IDC_CONTACTS_CONTACT_LIST));
				break;
			}

			case IDC_RESET_FILTERS_BUTTON:
			{
				DisposeFilteredContacts();
				ResetUI(hDlg);
				LoadContactsToListView(GetDlgItem(hDlg, IDC_CONTACTS_CONTACT_LIST));
				break;
			}

			case IDC_CONTACTS_ADD_CONTACT:
			{
				Contact newContact;
				if (ContactInputDialog::Show(hDlg, newContact, currentTheme))
				{
					if (contactBook->ContainsPhone(newContact.phone))
					{
						MessageBox(hDlg,
							L"Не вдалося додати контакт, оскільки такий номер вже існує у контактній книзі!",
							L"Контакт існує",
							MB_ICONINFORMATION);
					}
					else
					{
						contactBook->AddContact(newContact);
						LoadContactsToListView(GetDlgItem(hDlg, IDC_CONTACTS_CONTACT_LIST));
					}
				}
				break;
			}

			case IDC_CONTACTS_DELETE_CONTACT:
			{
				DeleteSelectedContact(hDlg);
				break;
			}

			case IDC_CONTACTS_FILTER:
			{
				FilterContactsByTag(hDlg);
				break;
			}

			case IDC_CONTACTS_TOGGLE_FAVORITE:
			{
				ToggleFavoriteContactStatus(hDlg);
				break;
			}

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

		case WM_CLOSE:
			Dispose(hDlg);
			EndDialog(hDlg, ID_CONTACTS_CLOSE);
			return TRUE;

		case WM_DESTROY:
			HotkeysControl::UnregisterDefaultMenuHotkeys(hDlg);
			if (hFont) DeleteObject(hFont);
			break;
		}
		return FALSE;
	}

	void ContactBookDialog::InitializeContactBookDialog(LPARAM lParam, HWND& hDlg)
	{
		currentTheme = (int)lParam;
		hFont = AppStyles::GetDefaultButtonFont();
		InitializeControls(hDlg);
		UIHelpers::ApplyTheme(hDlg, currentTheme, false);
		InitializeComboboxes(hDlg);

		if (!LoadContactBookFromFile(hDlg, lastFile, false))
		{
			// Get path to executable
			wchar_t exePath[MAX_PATH];
			GetModuleFileName(NULL, exePath, MAX_PATH);

			// Extract directory path
			wchar_t* lastSlash = wcsrchr(exePath, L'\\');
			if (lastSlash) *lastSlash = L'\0';

			// Create default filename
			std::wstring defaultFile = std::wstring(exePath) + L"\\contacts.txt";

			if (!LoadContactBookFromFile(hDlg, defaultFile, false))
			{
				if (contactBook->SaveToFile(defaultFile))
				{
					lastFile = defaultFile;
				}
			}
			else
			{
				lastFile = defaultFile;
			}
		}
		HotkeysControl::RegisterDefaultMenuHotkeys(hDlg);
	}

	void ContactBookDialog::ToggleFavoriteContactStatus(HWND hDlg)
	{
		HWND hList = GetDlgItem(hDlg, IDC_CONTACTS_CONTACT_LIST);
		int selectedIndex = ListView_GetNextItem(hList, -1, LVNI_SELECTED);

		if (selectedIndex != -1)
		{
			// Get the contact reference from the list view
			LVITEM lvi = { 0 };
			lvi.mask = LVIF_PARAM;
			lvi.iItem = selectedIndex;
			ListView_GetItem(hList, &lvi);

			Contact* pContact = reinterpret_cast<Contact*>(lvi.lParam);

			if (pContact)
			{
				// Toggle by reference
				contactBook->ToggleFavorite(*pContact);

				if (filteredContacts != nullptr)
				{
					FilterContactsByTag(hDlg);
				}

				LoadContactsToListView(hList);
			}
		}
		else
		{
			MessageBox(hDlg,
				L"Будь ласка, виберіть контакт для зміни статусу 'Обраний'",
				L"Не вибрано контакт",
				MB_ICONINFORMATION);
		}
	}

	void ContactBookDialog::ResetUI(HWND hDlg)
	{
		SetDlgItemText(hDlg, IDC_FILTER_EDIT, L"");
		SetDlgItemText(hDlg, IDC_FILTER_STATUS, L"Фільтрування за тегом");
	}

	void ContactBookDialog::FilterContactsByTag(HWND hDlg)
	{
		HWND hFilterEdit = GetDlgItem(hDlg, IDC_FILTER_EDIT);
		wchar_t filterText[256] = { 0 };
		GetWindowText(hFilterEdit, filterText, 256);


		DisposeFilteredContacts();

		filteredContacts = contactBook->FilterContacts(filterText, filteredContactsCount);

		// Refresh the list view
		LoadContactsToListView(GetDlgItem(hDlg, IDC_CONTACTS_CONTACT_LIST));

		// Show how many contacts are visible
		std::wstring status;

		if (filteredContacts != nullptr)
		{
			status = L"Знайдено контактів : " + std::to_wstring(filteredContactsCount);
		}
		else
		{
			status = L"Фільтрування за тегом";
		}

		SetDlgItemText(hDlg, IDC_FILTER_STATUS, status.c_str());
	}

	void ContactBookDialog::DeleteSelectedContact(HWND hDlg)
	{
		HWND hList = GetDlgItem(hDlg, IDC_CONTACTS_CONTACT_LIST);
		int selectedIndex = ListView_GetNextItem(hList, -1, LVNI_SELECTED);

		if (selectedIndex != -1)
		{
			LVITEM lvi = { 0 };
			lvi.mask = LVIF_PARAM;
			lvi.iItem = selectedIndex;
			ListView_GetItem(hList, &lvi);

			Contact* pContact = reinterpret_cast<Contact*>(lvi.lParam);

			if (pContact)
			{
				if (contactBook->DeleteContactByReference(*pContact))
				{
					// Refresh the list view
					if (filteredContacts != nullptr)
					{
						FilterContactsByTag(hDlg);
					}
					LoadContactsToListView(hList);
					MessageBox(hDlg, L"Контакт було успішно видалено.", L"Результат операції видалення", MB_OK);
				}
				else
				{
					MessageBox(hDlg, L"Контакт не було видалено, спробуйте ще раз.", L"Результат операції видалення", MB_ICONERROR);
				}
			}
		}
		else
		{
			MessageBox(hDlg, L"Для видалення контакту, необхідно обрати його з контактної книги.",
				L"Результат операції видалення", MB_ICONINFORMATION);
		}
	}

	void ContactBookDialog::Dispose(HWND& hDlg)
	{
		delete contactBook;
		contactBook = nullptr;
		DisposeFilteredContacts();
		ResetUI(hDlg);
	}

	void ContactBookDialog::DisposeFilteredContacts()
	{
		if (filteredContacts != nullptr)
		{
			delete[] filteredContacts;
			filteredContacts = nullptr;
			filteredContactsCount = 0;
		}
	}


	ContactBook::SortType ContactBookDialog::GetSortTypeFromColumnIndex(int columnIndex)
	{
		switch (columnIndex)
		{
		case 0: return ContactBook::SortType::Favorite;
		case 1: return ContactBook::SortType::Name;
		case 2: return ContactBook::SortType::Phone;
		case 3: return ContactBook::SortType::Country;
		case 4: return ContactBook::SortType::Address;
		case 5: return ContactBook::SortType::AddedTime;
		case 6: return ContactBook::SortType::Tags;
		default: return ContactBook::SortType::Name;
		}
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
					L"Неможливо зберегти в поточний файл, можливо він пошкоджений або не існує. Зберегти в новий файл?",
					L"Збереження контактної книги",
					MB_YESNO | MB_ICONWARNING);

				if (result == IDYES)
				{
					SaveContactBookAs(hDlg);
				}
			}
			else
			{
				MessageBox(hDlg,
					L"Контактна книга успішно збережена в поточний файл.",
					L"Збереження контактної книги",
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

		return TRUE;
	}

	void ContactBookDialog::SaveContactBookAs(HWND hDlg)
	{
		// Initialize Save As dialog
		OPENFILENAME ofn = { 0 };
		wchar_t* szFile = Utils::SelectSaveFile(hDlg, ofn, L"contacts.txt");

		// Show the Save As dialog
		if (GetSaveFileName(&ofn))
		{
			if (contactBook->SaveToFile(szFile))
			{
				MessageBox(hDlg,
					L"Контактна книга успішно збережена.",
					L"Збереження контактної книги",
					MB_OK | MB_ICONINFORMATION);
			}
			else
			{
				MessageBox(hDlg,
					L"Неможливо зберегти файл, можливо він пошкоджений або кінцева директорія не існує.",
					L"Збереження контактної книги",
					MB_OK | MB_ICONERROR);
			}
		}
	}


	void ContactBookDialog::LoadContactBookFromFile(HWND& hDlg)
	{
		Dispose(hDlg);
		contactBook = new ContactBook();

		// Initialize the OPENFILENAME structure
		OPENFILENAME ofn = { 0 };
		wchar_t* szFile = Utils::SelectLoadFile(hDlg, ofn);

		// Show the Open dialog
		if (GetOpenFileName(&ofn))
		{
			LoadContactBookFromFile(hDlg, szFile);
			lastFile.clear();
			lastFile.append(szFile);  // Store the file path
		}
	}


	bool ContactBookDialog::LoadContactBookFromFile(HWND& hDlg, std::wstring szFile, bool notifyAboutResult)
	{
		if (contactBook->LoadFromFile(szFile))
		{
			LoadContactsToListView(GetDlgItem(hDlg, IDC_CONTACTS_CONTACT_LIST));

			if (notifyAboutResult)
			{
				MessageBox(hDlg,
					L"Контактна книга успішно завантажена з файлу.",
					L"Завантаження контактної книги",
					MB_OK | MB_ICONINFORMATION);
			}

			return true;
		}
		else
		{
			if (notifyAboutResult)
			{
				MessageBox(hDlg,
					L"Неможливо завантажити файл контактної книги, можливо він пошкоджений або не існує",
					L"Завантаження контактної книги",
					MB_OK | MB_ICONERROR);
			}

			return false;
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

		const wchar_t* sortColumns[] = {
			L"Обраний", L"Ім'я", L"Телефон", L"Країна", L"Адреса", L"Дата додавання", L"Теги"
		};

		const wchar_t* copyColumns[] = {
			L"Ім'я", L"Телефон", L"Країна", L"Адреса", L"Дата додавання", L"Теги"
		};

		for (const wchar_t* col : sortColumns)
		{
			SendMessage(hSortCombo, CB_ADDSTRING, 0, (LPARAM)col);
		}

		for (const wchar_t* col : copyColumns)
		{
			SendMessage(hCopyCombo, CB_ADDSTRING, 0, (LPARAM)col);
		}

		SendMessage(hCopyCombo, CB_SETCURSEL, 0, 0);
		SendMessage(hSortCombo, CB_SETCURSEL, 0, 0);
	}

	void ContactBookDialog::InitializeControls(HWND hDlg)
	{
		// Initialize owner-draw buttons
		UIHelpers::InitOwnerDrawButton(hDlg, IDC_CONTACTS_ADD_CONTACT, hFont);
		UIHelpers::InitOwnerDrawButton(hDlg, IDC_CONTACTS_DELETE_CONTACT, hFont);
		UIHelpers::InitOwnerDrawButton(hDlg, IDC_CONTACTS_TOGGLE_FAVORITE, hFont);
		UIHelpers::InitOwnerDrawButton(hDlg, IDC_CONTACTS_FILTER, hFont);
		UIHelpers::InitOwnerDrawButton(hDlg, IDC_COPY_BUTTON, hFont);
		UIHelpers::InitOwnerDrawButton(hDlg, IDC_SORT_BUTTON, hFont);
		UIHelpers::InitOwnerDrawButton(hDlg, IDC_RESET_BUTTON, hFont);
		UIHelpers::InitOwnerDrawButton(hDlg, ID_CONTACTS_CLOSE, hFont);
		UIHelpers::InitOwnerDrawButton(hDlg, IDC_RESET_FILTERS_BUTTON, hFont);

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

		// Set the list view to Unicode mode
		ListView_SetUnicodeFormat(hList, TRUE);

		if (filteredContacts == nullptr)
		{
			const Contact* contacts = contactBook->GetContacts();
			size_t contactCount = contactBook->GetContactCount();
			LoadContactsToListView(hList, contacts, contactCount);
		}
		else
		{
			LoadContactsToListView(hList, filteredContacts, filteredContactsCount);
		}

		// Re-enable redraw and force a repaint
		SendMessage(hList, WM_SETREDRAW, TRUE, 0);
		RedrawWindow(hList, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
	}

	void ContactBookDialog::LoadContactsToListView(HWND& hList, const Contact* contacts, size_t contactCount)
	{
		for (size_t i = 0; i < contactCount; i++)
		{
			const Contact& c = contacts[i];

			LVITEM lvi = { 0 };
			lvi.mask = LVIF_TEXT | LVIF_PARAM;
			lvi.iItem = (int)i;
			lvi.lParam = (LPARAM)&contacts[i];

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
	}

	void ContactBookDialog::CopyInfoFromSelectedContact(HWND hDlg)
	{
		HWND hList = GetDlgItem(hDlg, IDC_CONTACTS_CONTACT_LIST);
		HWND hCombo = GetDlgItem(hDlg, IDC_COPY_COLUMN_COMBO);

		// Get selected contact
		int selectedItem = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
		if (selectedItem == -1)
		{
			MessageBox(hDlg, L"Спочатку необхідно обрати контакт для копіювання", L"Контакт не обраний", MB_ICONINFORMATION);
			return;
		}

		// Get selected column from combo box
		int selectedColumn = (int)SendMessage(hCombo, CB_GETCURSEL, 0, 0);
		if (selectedColumn == CB_ERR)
		{
			MessageBox(hDlg, L"Спочатку необхідно обрати колонку яку необхідно скопіювати!", L"Колонка не обрана", MB_ICONINFORMATION);
			return;
		}

		int columnIndex = selectedColumn + 1; // +1 to skip favorite column

		// Get the text from the selected cell
		wchar_t buffer[256] = { 0 };
		ListView_GetItemText(hList, selectedItem, columnIndex, buffer, ARRAYSIZE(buffer));

		// Check if the target column is empty
		if (wcslen(buffer) == 0)
		{
			MessageBox(hDlg, L"Інформація не була скопійована, оскільки колонка порожня.", L"Порожня колонка", MB_ICONWARNING);
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
}
