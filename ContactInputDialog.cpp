#include "ContactInputDialog.h"
#include "AppStyles.h"
#include "UIHelpers.h"
#include "Utils.h"

namespace ContactBookControl
{
	const int DEFAULT_COUNTRIES_COUNT = sizeof(DEFAULT_COUNTRIES) / sizeof(DEFAULT_COUNTRIES[0]);

	bool ContactInputDialog::Show(HWND hParent, Contact& contact, int theme)
	{
		ContactInputDialogData data = { &contact, theme };
		return DialogBoxParam(GetModuleHandle(NULL),
			MAKEINTRESOURCE(IDD_CONTACT_INPUT_DIALOG),
			hParent,
			DialogProc,
			reinterpret_cast<LPARAM>(&data)) == ID_CONTACT_CONFIRM;
	}

	INT_PTR CALLBACK ContactInputDialog::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
		static ContactInputDialogData* pData = nullptr;
		static HFONT hFont = nullptr;

		switch (message)
		{
		case WM_INITDIALOG:
		{
			pData = reinterpret_cast<ContactInputDialogData*>(lParam);
			if (pData && pData->contact)
			{
				// Create font
				HDC hdc = GetDC(hDlg);
				int fontSize = -MulDiv(9, GetDeviceCaps(hdc, LOGPIXELSY), 72);
				ReleaseDC(hDlg, hdc);
				hFont = AppStyles::GetDefaultButtonFont();

				// Initialize controls
				SetDlgItemTextW(hDlg, IDC_CONTACT_NAME, pData->contact->name.c_str());
				SetDlgItemTextW(hDlg, IDC_CONTACT_PHONE, pData->contact->phone.c_str());
				SetDlgItemTextW(hDlg, IDC_CONTACT_TAGS, pData->contact->tags.c_str());

				// Initialize country combo
				HWND hCountryCombo = GetDlgItem(hDlg, IDC_CONTACT_COUNTRY_COMBO);
				for (int i = 0; i < DEFAULT_COUNTRIES_COUNT; ++i)
				{
					SendMessageA(hCountryCombo, CB_ADDSTRING, 0, (LPARAM)DEFAULT_COUNTRIES[i]);
				}

				if (!pData->contact->country.empty())
				{
					LRESULT index = SendMessageA(hCountryCombo, CB_FINDSTRINGEXACT, -1,
						(LPARAM)pData->contact->country.c_str());
					if (index != CB_ERR)
					{
						SendMessageA(hCountryCombo, CB_SETCURSEL, index, 0);
					}
					EnableAddressField(hDlg, true);
					SetDlgItemTextW(hDlg, IDC_CONTACT_ADDRESS, pData->contact->address.c_str());
				}

				// Apply theme and initialize buttons
				UIHelpers::ApplyTheme(hDlg, pData->theme);
				InitializeControls(hDlg);
			}
			return TRUE;
		}

		case WM_DRAWITEM:
			if (UIHelpers::DrawThemedButton((LPDRAWITEMSTRUCT)lParam, pData ? pData->theme : 0))
			{
				return TRUE;
			}
			break;

		case WM_CTLCOLORDLG:
			if (pData)
			{
				return (LRESULT)CreateSolidBrush(AppStyles::GetThemeColors(pData->theme).windowBg);
			}
			break;

		case WM_CTLCOLORSTATIC:
		case WM_CTLCOLORBTN:
		{
			if (pData)
			{
				HDC hdc = (HDC)wParam;
				SetTextColor(hdc, AppStyles::GetThemeColors(pData->theme).titleText);
				SetBkMode(hdc, TRANSPARENT);
				return (LRESULT)CreateSolidBrush(AppStyles::GetThemeColors(pData->theme).windowBg);
			}
			break;
		}


		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDC_COUNTRY_SEARCH:
				if (HIWORD(wParam) == EN_CHANGE)
				{
					char searchText[256];
					GetDlgItemTextA(hDlg, IDC_COUNTRY_SEARCH, searchText, sizeof(searchText));
					FilterCountries(hDlg, searchText);
				}
				break;

			case IDC_CONTACT_COUNTRY_COMBO:
				if (HIWORD(wParam) == CBN_SELCHANGE)
				{
					HWND hCombo = GetDlgItem(hDlg, IDC_CONTACT_COUNTRY_COMBO);
					LRESULT sel = SendMessage(hCombo, CB_GETCURSEL, 0, 0);
					EnableAddressField(hDlg, sel != CB_ERR);
				}
				break;

			case ID_CONTACT_CONFIRM:
			{
				if (!pData || !pData->contact)
				{
					EndDialog(hDlg, ID_CONTACT_CANCEL);
					return TRUE;
				}

				const int bufferSize = 256;
				wchar_t buffer[bufferSize];

				// Validate name
				GetDlgItemTextW(hDlg, IDC_CONTACT_NAME, buffer, sizeof(buffer));
				if (wcslen(buffer) == 0)
				{
					MessageBoxA(hDlg, "Будь-ласка введіть ім'я", "Помилка", MB_ICONERROR);
					return FALSE;
				}
				pData->contact->name = buffer;

				// Validate phone
				GetDlgItemTextW(hDlg, IDC_CONTACT_PHONE, buffer, sizeof(buffer));
				char* phoneNumberBuffer = new char[bufferSize];
				WideCharToMultiByte(CP_UTF8, 0, buffer, -1, phoneNumberBuffer, bufferSize, nullptr, nullptr);
				if (!ValidatePhoneNumber(phoneNumberBuffer))
				{
					MessageBoxA(hDlg, "Будь-ласка введіть коректний номер телефону\nФормат: +[Код країни] [Номер] або [Номер з локальним кодом]",
						"Помилка", MB_ICONERROR);
					return FALSE;
				}
				pData->contact->phone = buffer;

				// Get country
				HWND hCombo = GetDlgItem(hDlg, IDC_CONTACT_COUNTRY_COMBO);
				LRESULT sel = SendMessage(hCombo, CB_GETCURSEL, 0, 0);
				if (sel != CB_ERR)
				{
					SendMessageW(hCombo, CB_GETLBTEXT, sel, (LPARAM)buffer);
					pData->contact->country = buffer;
				}
				else
				{
					pData->contact->country.clear();
				}

				// Get address if country is selected
				if (!pData->contact->country.empty())
				{
					GetDlgItemTextW(hDlg, IDC_CONTACT_ADDRESS, buffer, sizeof(buffer));
					pData->contact->address = buffer;
				}
				else
				{
					pData->contact->address.clear();
				}

				// Get tags
				GetDlgItemTextW(hDlg, IDC_CONTACT_TAGS, buffer, sizeof(buffer));
				pData->contact->tags = buffer;
				GetLocalTime(&pData->contact->addedDate);
				EndDialog(hDlg, ID_CONTACT_CONFIRM);
				return TRUE;
			}

			case IDCANCEL:
			case ID_CONTACT_CANCEL:
				EndDialog(hDlg, ID_CONTACT_CANCEL);
				return TRUE;
			}
			break;
		}

		case WM_DESTROY:
			if (hFont)
			{
				DeleteObject(hFont);
				hFont = nullptr;
			}
			break;
		}
		return FALSE;
	}

	void ContactInputDialog::InitializeControls(HWND hDlg)
	{
		UIHelpers::InitOwnerDrawButton(hDlg, ID_CONTACT_CONFIRM, AppStyles::GetDefaultButtonFont());
		UIHelpers::InitOwnerDrawButton(hDlg, ID_CONTACT_CANCEL, AppStyles::GetDefaultButtonFont());
	}


	bool ContactInputDialog::ValidatePhoneNumber(const std::string& phone)
	{
		std::string digits;
		for (char c : phone)
		{
			if (isdigit(c)) digits += c;
		}

		if (digits.length() < 7 || digits.length() > 15)
		{
			return false;
		}

		if (phone.find('+') == 0)
		{
			size_t plusPos = phone.find('+');
			size_t firstDigit = phone.find_first_of("0123456789", plusPos + 1);
			if (firstDigit == std::string::npos || firstDigit - plusPos > 4)
			{
				return false;
			}
		}

		return true;
	}

	void ContactInputDialog::PopulateCountries(HWND hCombo)
	{
		SendMessage(hCombo, CB_RESETCONTENT, 0, 0);
		for (int i = 0; i < DEFAULT_COUNTRIES_COUNT; ++i)
		{
			SendMessageA(hCombo, CB_ADDSTRING, 0, (LPARAM)DEFAULT_COUNTRIES[i]);
		}
	}

	void ContactInputDialog::FilterCountries(HWND hDlg, const std::string& filter)
	{
		HWND hCombo = GetDlgItem(hDlg, IDC_CONTACT_COUNTRY_COMBO);
		HWND hCountLabel = GetDlgItem(hDlg, IDC_CONTACT_COUNTRY_COUNT);
		SendMessage(hCombo, CB_RESETCONTENT, 0, 0);

		int matchCount = 0;

		// Convert the filter to lowercase
		std::string lowerFilter = Utils::ToLowerCase(filter);

		if (filter.empty())
		{
			PopulateCountries(hCombo);
			EnableWindow(hCombo, TRUE); // Enable the combo if filter is empty
			SetWindowTextA(hCountLabel, "");
			return;
		}

		for (int i = 0; i < DEFAULT_COUNTRIES_COUNT; ++i)
		{
			std::string country = DEFAULT_COUNTRIES[i];
			// Convert the country name to lowercase for comparison
			std::string lowerCountry = Utils::ToLowerCase(country);

			// Check if the lowercase country contains the lowercase filter string
			if (lowerCountry.find(lowerFilter) != std::string::npos)
			{
				SendMessageA(hCombo, CB_ADDSTRING, 0, (LPARAM)DEFAULT_COUNTRIES[i]);
				matchCount++;
			}
		}

		if (matchCount == 0)
		{
			EnableWindow(hCombo, FALSE); // Disable the combo if no matches
			SetWindowTextA(hCountLabel, "No countries found");
		}
		else
		{
			EnableWindow(hCombo, TRUE); // Enable the combo if matches found
			std::string countMessage = "Countries found: " + std::to_string(matchCount);
			SetWindowTextA(hCountLabel, countMessage.c_str());
		}
	}


	void ContactInputDialog::EnableAddressField(HWND hDlg, bool enable)
	{
		EnableWindow(GetDlgItem(hDlg, IDC_CONTACT_ADDRESS), enable);
		if (enable)
		{
			SetWindowTextA(GetDlgItem(hDlg, IDC_CONTACT_ADDRESS), "");
		}
		else
		{
			SetWindowTextA(GetDlgItem(hDlg, IDC_CONTACT_ADDRESS), "Select a country first");
		}
	}
}