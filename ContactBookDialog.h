#pragma once
#include <windows.h>
#include "ContactBook.h"
#include "UIHelpers.h"
#include "AppStyles.h"
#include "MenuHelpers.h"

class ContactBookDialog
{
public:
    static void Show(HWND parent, HINSTANCE hInst, int theme);

private:
    static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK InstructionsProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static void RegisterHotkeys(HWND hDlg);
    static void UnregisterHotkeys(HWND hDlg);
    static void InitializeControls(HWND hDlg);
    static void InitializeComboboxes(HWND hDlg);

    static void LoadContactBookFromFile(HWND& hDlg);
    static INT_PTR HandleConcreteMenuCommand(WPARAM& wParam, HWND& hDlg);
    static void CopyInfoFromSelectedContact(HWND hDlg);
    static void SaveContactBookAs(HWND hDlg);
    static void LoadContactsToListView(HWND hList);
    static Contact* GetContactFromListViewItem(HWND hList, int itemIndex);

    static HFONT hFont;
    static int currentTheme;
    static ContactBook* contactBook;
};