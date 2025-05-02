#pragma once
#include "ContactBook.h"
#include <string>
#include <commctrl.h>
#include "resource.h"
#include <algorithm>
#include "DefaultDataSets.h"

class ContactInputDialog
{
public:

	struct ContactInputDialogData
	{
		Contact* contact;
		int theme;
	};

	static bool Show(HWND hParent, Contact& contact, int theme);
	static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
	static void InitializeControls(HWND hDlg);
	static bool ValidatePhoneNumber(const std::string& phone);
	static void PopulateCountries(HWND hCombo);
	static void FilterCountries(HWND hDlg, const std::string& filter);
	static void EnableAddressField(HWND hDlg, bool enable);
};
