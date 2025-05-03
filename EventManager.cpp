#include "EventManager.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include "UIHelpers.h"
#include "Utils.h"

void EventsManager::Initialize()
{
    // Set data file path next to executable
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    std::wstring path(exePath);
    size_t lastSlash = path.find_last_of(L"\\/");
    dataFilePath = path.substr(0, lastSlash + 1) + L"events.txt";

    // Load existing events
    LoadEvents(dataFilePath);
}

void EventsManager::AddEvent(const Event& event)
{
    events.push_back(event);
    UpdateSingleEventStatus(events.back());
    SaveEvents();
}

void EventsManager::DeleteEvent(size_t index)
{
    if (index < events.size())
    {
        events.erase(events.begin() + index);
        SaveEvents();
    }
}

void EventsManager::MarkCompleted(size_t index, bool completed)
{
    if (index < events.size() && !events[index].isPastDue)
    {
        events[index].isCompleted = completed;
        SaveEvents();
    }
}

void EventsManager::ClearPastDueEvents()
{
    events.erase(
        std::remove_if(events.begin(), events.end(),
            [](const Event& e) { return e.isPastDue; }),
        events.end());
    SaveEvents();
}

void EventsManager::UpdateEventsStatus()
{
    for (auto& event : events)
    {
        UpdateSingleEventStatus(event);
    }
    SaveEvents();
}

void EventsManager::ProcessRecurringEvents()
{
    SYSTEMTIME now;
    GetLocalTime(&now);

    std::vector<Event> newEvents;

    for (auto& event : events)
    {
        if (event.isRecurring && event.isPastDue && !event.isCompleted)
        {
            // Calculate next occurrence
            SYSTEMTIME nextDate = event.deadline;

            // Add recurrence interval days
            FILETIME ft;
            SystemTimeToFileTime(&nextDate, &ft);

            ULARGE_INTEGER uli;
            uli.LowPart = ft.dwLowDateTime;
            uli.HighPart = ft.dwHighDateTime;

            // Add interval in 100-nanosecond intervals (1 day = 864000000000)
            uli.QuadPart += event.recurrenceInterval * 864000000000ULL;

            ft.dwLowDateTime = uli.LowPart;
            ft.dwHighDateTime = uli.HighPart;
            FileTimeToSystemTime(&ft, &nextDate);

            // Check if next occurrence is in future
            FILETIME nowFt, nextFt;
            SystemTimeToFileTime(&now, &nowFt);
            SystemTimeToFileTime(&nextDate, &nextFt);

            if (CompareFileTime(&nextFt, &nowFt) > 0)
            {
                Event newEvent = event;
                newEvent.deadline = nextDate;
                newEvent.isCompleted = false;
                newEvent.isPastDue = false;
                newEvents.push_back(newEvent);
            }
        }
    }

    events.insert(events.end(), newEvents.begin(), newEvents.end());
    SaveEvents();
}


void EventsManager::SortEvents(SortCriteria criteria, bool ascending)
{
    int result = 0;

    // Comparison function for sorting
    for (size_t i = 0; i < events.size(); ++i)
    {
        for (size_t j = i + 1; j < events.size(); ++j)
        {
            bool swap = false;

            switch (criteria)
            {
            case SortCriteria::Name:
                result = events[i].name.compare(events[j].name);
                break;

            case SortCriteria::Deadline:
                result = Utils::CompareSystemTimes(events[i].deadline, events[j].deadline);
                break;

            case SortCriteria::Importance:
                result =  events[j].importance - events[i].importance;
                break;

            case SortCriteria::CreationDate:
                result = Utils::CompareSystemTimes(events[j].creationDate, events[i].creationDate);
                break;
            }

            // Determine if we should swap
            if ((ascending && result > 0) || (!ascending && result < 0))
            {
                swap = true;
            }

            // Swap if necessary
            if (swap)
            {
                Event temp = events[i];
                events[i] = events[j];
                events[j] = temp;
            }
        }
    }
}

std::vector<Event> EventsManager::SearchByName(const std::wstring& name)
{
    std::vector<Event> results;
    std::wstring lowerName = name;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

    for (const auto& event : events)
    {
        std::wstring lowerEventName = event.name;
        std::transform(lowerEventName.begin(), lowerEventName.end(), lowerEventName.begin(), ::tolower);

        if (lowerEventName.find(lowerName) != std::wstring::npos)
        {
            results.push_back(event);
        }
    }

    return results;
}

std::vector<Event> EventsManager::SearchByDate(const SYSTEMTIME& date)
{
    std::vector<Event> results;

    for (const auto& event : events)
    {
        if (event.deadline.wYear == date.wYear &&
            event.deadline.wMonth == date.wMonth &&
            event.deadline.wDay == date.wDay)
        {
            results.push_back(event);
        }
    }

    return results;
}

EventsManager::EventStats EventsManager::GetWeeklyStats()
{
    EventStats stats = { 0, 0, 0.0f };
    SYSTEMTIME now;
    GetLocalTime(&now);
    FILETIME nowFt;
    SystemTimeToFileTime(&now, &nowFt);

    ULARGE_INTEGER uliNow;
    uliNow.LowPart = nowFt.dwLowDateTime;
    uliNow.HighPart = nowFt.dwHighDateTime;

    for (const auto& event : events)
    {
        FILETIME eventFt;
        SystemTimeToFileTime(&event.creationDate, &eventFt);

        ULARGE_INTEGER uliEvent;
        uliEvent.LowPart = eventFt.dwLowDateTime;
        uliEvent.HighPart = eventFt.dwHighDateTime;

        // Check if event was created in the last 7 days
        if ((uliNow.QuadPart - uliEvent.QuadPart) <= 7 * 864000000000ULL)
        {
            stats.total++;
            if (event.isCompleted)
            {
                stats.completed++;
            }
        }
    }

    if (stats.total > 0)
    {
        stats.completionRate = (static_cast<float>(stats.completed) / stats.total) * 100.0f;
    }

    return stats;
}

EventsManager::EventStats EventsManager::GetMonthlyStats()
{
    EventStats stats = { 0, 0, 0.0f };
    SYSTEMTIME now;
    GetLocalTime(&now);

    for (const auto& event : events)
    {
        if (event.creationDate.wYear == now.wYear &&
            event.creationDate.wMonth == now.wMonth)
        {
            stats.total++;
            if (event.isCompleted)
            {
                stats.completed++;
            }
        }
    }

    if (stats.total > 0)
    {
        stats.completionRate = (static_cast<float>(stats.completed) / stats.total) * 100.0f;
    }

    return stats;
}

void EventsManager::CheckAlarms()
{
    SYSTEMTIME now;
    GetLocalTime(&now);
    FILETIME nowFt;
    SystemTimeToFileTime(&now, &nowFt);

    for (const auto& event : events)
    {
        if (event.hasAlarm && !event.isCompleted && !event.isPastDue)
        {
            FILETIME eventFt;
            SystemTimeToFileTime(&event.deadline, &eventFt);

            // Check if deadline is within the next 15 minutes
            ULARGE_INTEGER uliNow, uliEvent;
            uliNow.LowPart = nowFt.dwLowDateTime;
            uliNow.HighPart = nowFt.dwHighDateTime;
            uliEvent.LowPart = eventFt.dwLowDateTime;
            uliEvent.HighPart = eventFt.dwHighDateTime;

            if ((uliEvent.QuadPart - uliNow.QuadPart) <= 15 * 60 * 10000000ULL)
            {
                // Trigger alarm (in a real app, this would show a notification)
                std::wstring message = L"Нагадування: " + event.name + L"\nЧас: " +
                    std::to_wstring(event.deadline.wHour) + L":" +
                    std::to_wstring(event.deadline.wMinute);
                MessageBoxW(NULL, message.c_str(), L"Нагадування про подію", MB_ICONINFORMATION);
            }
        }
    }
}

// Load events from a given file path
bool EventsManager::LoadEvents(const std::wstring& filePath)
{
    std::wifstream file(filePath, std::ios::binary);
    if (!file) return false;

    events.clear();
    dataFilePath = filePath;  // Save the last loaded file path

    std::wstring line;
    while (std::getline(file, line))
    {
        if (line.empty()) continue;

        std::wistringstream iss(line);
        Event event;
        wchar_t sep;

        // Read basic fields
        std::getline(iss, event.name, L'|');
        std::getline(iss, event.description, L'|');
        std::getline(iss, event.notes, L'|');
        iss >> event.importance >> sep;
        iss >> event.hasAlarm >> sep;
        iss >> event.isRecurring >> sep;
        iss >> event.recurrenceInterval >> sep;
        iss >> event.isCompleted >> sep;
        iss >> event.isPastDue >> sep;

        // Read creation date
        iss >> event.creationDate.wYear >> sep;
        iss >> event.creationDate.wMonth >> sep;
        iss >> event.creationDate.wDay >> sep;
        iss >> event.creationDate.wHour >> sep;
        iss >> event.creationDate.wMinute >> sep;

        // Read deadline
        iss >> event.deadline.wYear >> sep;
        iss >> event.deadline.wMonth >> sep;
        iss >> event.deadline.wDay >> sep;
        iss >> event.deadline.wHour >> sep;
        iss >> event.deadline.wMinute;

        events.push_back(event);
    }

    return true;
}

// Save events to the last loaded file path (uses dataFilePath)
bool EventsManager::SaveEvents()
{
    return SaveEvents(dataFilePath);
}

// Save events to a specific file path
bool EventsManager::SaveEvents(const std::wstring& filePath)
{
    std::wofstream file(filePath, std::ios::binary);
    if (!file) return false;

    for (const auto& event : events)
    {
        file << event.name << L"|"
            << event.description << L"|"
            << event.notes << L"|"
            << event.importance << L"|"
            << event.hasAlarm << L"|"
            << event.isRecurring << L"|"
            << event.recurrenceInterval << L"|"
            << event.isCompleted << L"|"
            << event.isPastDue << L"|"
            << event.creationDate.wYear << L"|"
            << event.creationDate.wMonth << L"|"
            << event.creationDate.wDay << L"|"
            << event.creationDate.wHour << L"|"
            << event.creationDate.wMinute << L"|"
            << event.deadline.wYear << L"|"
            << event.deadline.wMonth << L"|"
            << event.deadline.wDay << L"|"
            << event.deadline.wHour << L"|"
            << event.deadline.wMinute << L"\n";
    }

    // Optionally, update the last loaded file path after saving
    dataFilePath = filePath;

    return true;
}


const std::vector<Event>& EventsManager::GetAllEvents()
{
    return events;
}

bool EventsManager::IsPastDue(const Event& event)
{
    SYSTEMTIME now;
    GetLocalTime(&now);

    FILETIME nowFt, deadlineFt;
    SystemTimeToFileTime(&now, &nowFt);
    SystemTimeToFileTime(&event.deadline, &deadlineFt);

    return CompareFileTime(&nowFt, &deadlineFt) > 0;
}

void EventsManager::UpdateSingleEventStatus(Event& event)
{
    if (!event.isCompleted)
    {
        event.isPastDue = IsPastDue(event);
    }
}