#pragma once
#include "Event.h"
#include <vector>
#include <string>

class EventsManager
{
public:
    // Event management
    void AddEvent(const Event& event);
    void DeleteEvent(size_t index);
    void MarkCompleted(size_t index, bool completed);
    void ClearPastDueEvents();
    void UpdateEventsStatus();

    // Recurring events
    void ProcessRecurringEvents();

    // Sorting
    enum SortCriteria { Name, Deadline, Importance, CreationDate };
    void SortEvents(SortCriteria criteria, bool ascending = true);

    // Searching
    std::vector<Event> SearchByName(const std::wstring& name);
    std::vector<Event> SearchByDate(const SYSTEMTIME& date);

    // Statistics
    struct EventStats
    {
        int total;
        int completed;
        float completionRate;
    };
    EventStats GetWeeklyStats();
    EventStats GetMonthlyStats();

    // Persistence
    bool LoadEvents(const std::wstring& filePath);
    bool SaveEvents();
    bool SaveEvents(const std::wstring& filePath);

    // Alarm
    void CheckAlarms();

    // Access
    const std::vector<Event>& GetAllEvents();

private:
    std::vector<Event> events;
    std::wstring dataFilePath;

    void Initialize();
    bool IsPastDue(const Event& event);
    void UpdateSingleEventStatus(Event& event);
};