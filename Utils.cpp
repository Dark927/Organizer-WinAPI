#define UNICODE
#define _UNICODE

#include "Utils.h"
#include <sstream>
#include <ctime>
#include <algorithm>

// Helper function to convert a string to lowercase
std::string Utils::ToLowerCase(const std::string& input)
{
	std::string result = input;
	std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });
	return result;
}


int Utils::CompareSystemTimes(const SYSTEMTIME& a, const SYSTEMTIME& b)
{
    // Compare the year
    if (a.wYear < b.wYear) return -1;
    if (a.wYear > b.wYear) return 1;

    // Compare the month
    if (a.wMonth < b.wMonth) return -1;
    if (a.wMonth > b.wMonth) return 1;

    // Compare the day
    if (a.wDay < b.wDay) return -1;
    if (a.wDay > b.wDay) return 1;

    // Compare the hour
    if (a.wHour < b.wHour) return -1;
    if (a.wHour > b.wHour) return 1;

    // Compare the minute
    if (a.wMinute < b.wMinute) return -1;
    if (a.wMinute > b.wMinute) return 1;

    // Compare the second
    if (a.wSecond < b.wSecond) return -1;
    if (a.wSecond > b.wSecond) return 1;

    // Compare the milliseconds
    if (a.wMilliseconds < b.wMilliseconds) return -1;
    if (a.wMilliseconds > b.wMilliseconds) return 1;

    // If all fields are equal, return 0
    return 0;
}

int Utils::CalculateMinutesDifference(const SYSTEMTIME& earlier, const SYSTEMTIME& later)
{
    ULONGLONG earlierMinutes = TimeToMinutes(earlier);
    ULONGLONG laterMinutes = TimeToMinutes(later);

    return static_cast<int>(laterMinutes - earlierMinutes);
}

ULONGLONG Utils::TimeToMinutes(const SYSTEMTIME& st)
{
    // Calculate total minutes since reference point (year 1601 like FILETIME)
    // Note: This is a simplified calculation and might need adjustment for leap years
    // and other calendar complexities for precise long-term calculations

    // Years to days
    ULONGLONG totalDays = (st.wYear - 1601) * 365;
    // Add leap days (simplified)
    totalDays += (st.wYear - 1601) / 4;

    // Months to days
    static const int monthDays[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    for (int m = 1; m < st.wMonth; m++)
    {
        totalDays += monthDays[m];
    }
    // Add current day
    totalDays += st.wDay - 1;

    // Days to hours
    ULONGLONG totalHours = totalDays * 24 + st.wHour;

    // Hours to minutes
    return totalHours * 60 + st.wMinute;
}

SYSTEMTIME Utils::AddMinutesToSystemTime(SYSTEMTIME time, int minutes)
{
    // Add minutes directly
    time.wMinute += minutes;

    // Normalize the time structure by handling overflow
    while (time.wMinute >= 60)
    {
        time.wMinute -= 60;
        time.wHour++;
    }

    while (time.wHour >= 24)
    {
        time.wHour -= 24;
        time.wDay++;
    }

    // Handle month overflow (considering different month lengths)
    while (time.wDay > DaysInMonth(time.wYear, time.wMonth))
    {
        time.wDay -= DaysInMonth(time.wYear, time.wMonth);
        time.wMonth++;
    }

    while (time.wMonth > 12)
    {
        time.wMonth -= 12;
        time.wYear++;
    }

    return time;
}

int Utils::DaysInMonth(int year, int month)
{
    static const int daysPerMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    if (month == 2 && IsLeapYear(year))
    {
        return 29;
    }

    if (month >= 1 && month <= 12)
    {
        return daysPerMonth[month - 1];
    }

    return 0;
}

bool Utils::IsLeapYear(int year)
{
    if (year % 4 != 0)
    {
        return false;
    }
    else if (year % 100 != 0)
    {
        return true;
    }
    else
    {
        return (year % 400 == 0);
    }
}


wchar_t* Utils::SelectLoadFile(HWND hDlg, OPENFILENAME& ofn)
{
    // Buffer to store the file path
    static wchar_t szFile[MAX_PATH] = { 0 }; // Static buffer

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hDlg;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrDefExt = L"txt";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    return szFile;
}

wchar_t* Utils::SelectSaveFile(HWND hDlg, OPENFILENAME& ofn, const wchar_t* defaultFileName)
{
    // Static buffer for file path
    static wchar_t szFile[MAX_PATH] = { 0 };
    wcscpy_s(szFile, MAX_PATH, defaultFileName); // Default filename

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hDlg;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrDefExt = L"txt";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

    return szFile;
}

bool Utils::IsDateTimeInPast(SYSTEMTIME targetDateTime)
{
    // Get current time
    SYSTEMTIME currentTime;
    GetLocalTime(&currentTime);

    return Utils::CompareSystemTimes(targetDateTime, currentTime) <= 0;
}


