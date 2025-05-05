#pragma once
#include <windows.h>
#include <string>
#include <vector>

enum AlarmState
{
    None,
    OneHourBefore,
    ThirtyMinutesBefore,
    FifteenMinutesBefore,
    ThreeMinutesBefore,
    Overdue
};

enum NotificationState
{
    NotNotified,
    OneHourNotified,
    ThirtyMinutesNotified,
    FifteenMinutesNotified,
    ThreeMinutesNotified
};

struct Event
{
    NotificationState notificationState = NotificationState::NotNotified;
    std::wstring name;
    std::wstring description;
    std::wstring notes;
    SYSTEMTIME creationDate;
    SYSTEMTIME targetDateTime;
    int importance; // 1-5 stars
    bool hasAlarm;
    bool isRecurring;
    bool recurringWasCreated;
    int recurrenceInterval; // in days
    bool isPastDue;

    // Constructor
    Event() :
        importance(3),
        hasAlarm(true),
        isRecurring(false), 
        recurringWasCreated(false), 
        recurrenceInterval(1),
        isPastDue(false),
        name(L""),
        description(L""), 
        notes(L"")
    {
    }

};