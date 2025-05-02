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
    static void InitializeControls(HWND hDlg);
    static void LoadContacts(HWND hList);
    static void InitializeComboboxes(HWND hDlg);

    static HFONT hFont;
    static int currentTheme;
    static ContactBook* contactBook;
};