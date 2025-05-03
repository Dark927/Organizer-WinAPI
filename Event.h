#pragma once
#include <windows.h>
#include <string>
#include <vector>

struct Event
{
    std::wstring name;
    std::wstring description;
    std::wstring notes;
    SYSTEMTIME creationDate;
    SYSTEMTIME deadline;
    int importance; // 1-5 stars
    bool isCompleted;
    bool hasAlarm;
    bool isRecurring;
    int recurrenceInterval; // in days
    bool isPastDue;

    // Constructor
    Event() : importance(3), isCompleted(false), hasAlarm(true),
        isRecurring(false), recurrenceInterval(1), isPastDue(false), name(L""), description(L""), notes(L"")
    {
    }

};