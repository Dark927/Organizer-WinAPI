#pragma once
#include <windows.h>
#include "ContactBook.h"
#include "UIHelpers.h"
#include "AppStyles.h"
#include "MenuHelpers.h"

namespace ContactBookControl
{
	class ContactBookDialog
	{
	public:
		static void Show(HWND parent, HINSTANCE hInst, int theme);

	private:
		static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
		static INT_PTR CALLBACK InstructionsProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
		static void InitializeControls(HWND hDlg);
		static void InitializeComboboxes(HWND hDlg);

		static INT_PTR HandleConcreteMenuCommand(WPARAM& wParam, HWND& hDlg);
		static void InitializeContactBookDialog(LPARAM lParam, HWND& hDlg);
		static void DeleteSelectedContact(HWND hDlg);
		static void CopyInfoFromSelectedContact(HWND hDlg);
		static void SaveContactBookAs(HWND hDlg);
		static void LoadContactBookFromFile(HWND& hDlg);
		static void LoadContactsToListView(HWND hList);
		static void LoadContactsToListView(HWND& hList, const Contact* contacts, size_t contactCount);
		static bool LoadContactBookFromFile(HWND& hDlg, std::wstring szFile, bool notifyAboutResult = true);
		static void Dispose(HWND& hDlg);
		static void DisposeFilteredContacts();
		static void ResetUI(HWND hDlg);
		static void FilterContactsByTag(HWND hDlg);
		static ContactBook::SortType GetSortTypeFromColumnIndex(int columnIndex);
		static void ToggleFavoriteContactStatus(HWND hDlg);

		static HFONT hFont;
		static int currentTheme;
		static ContactBook* contactBook;
	};
}