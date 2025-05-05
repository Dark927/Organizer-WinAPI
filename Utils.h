#pragma once
#include <string>
#include "Windows.h"

class Utils
{
public:
	static std::string ToLowerCase(const std::string& input);

	static int CompareSystemTimes(const SYSTEMTIME& a, const SYSTEMTIME& b);
	static wchar_t* SelectLoadFile(HWND hDlg, OPENFILENAME& ofn);
	static wchar_t* SelectSaveFile(HWND hDlg, OPENFILENAME& ofn, const wchar_t* defaultFileName = L"data.txt");
	static bool IsDateTimeInPast(SYSTEMTIME targetDateTime);

	static ULONGLONG TimeToMinutes(const SYSTEMTIME& st);
	static int CalculateMinutesDifference(const SYSTEMTIME& earlier, const SYSTEMTIME& later);
	static int DaysInMonth(int year, int month);
	static bool IsLeapYear(int year);
	static SYSTEMTIME AddMinutesToSystemTime(SYSTEMTIME time, int minutes);
};


