#pragma once
#include "Event.h"

namespace EventsManagerControl
{
    struct AlarmNotification
    {
        const Event* event;
        AlarmState state;
    };

    class EventsManager
    {
    public:
        EventsManager();
        ~EventsManager();

        // Event management
        void AddEvent(const Event& event);
        void DeleteEvent(size_t index);
        void ClearPastDueEvents();
        void UpdateEventsStatus();

        // Recurring events
        void ProcessRecurringEvents();

        // Sorting
        enum SortCriteria { Name, Deadline, Importance, CreationDate };
        void SortEvents(SortCriteria criteria, bool ascending = true);

        // Persistence
        bool LoadFromFile(const std::wstring& filePath);
        void UpdateEvent(size_t index, const Event& updatedEvent);
        bool SaveToCurrentFile();
        bool SaveToFile(const std::wstring& filePath);
        void CommitChanges();
        void DiscardChanges();

        // Alarm notifications
        void CheckForAlarms();
        const AlarmNotification* GetActiveAlarms() const;
        size_t GetActiveAlarmCount() const;
        void ClearAlarmNotifications();

        // Access
        const Event* GetCurrentEvents() const;
        const Event* GetSavedEvents() const;
        size_t GetCurrentEventCount() const;
        size_t GetSavedEventCount() const;

    private:
        Event* savedEvents;
        Event* currentEvents;
        size_t savedEventCount;
        size_t currentEventCount;
        size_t eventCapacity;
        std::wstring dataFilePath;

        AlarmNotification* activeAlarms;
        size_t activeAlarmCount;
        size_t activeAlarmCapacity;

        void Initialize();
        void UpdateSingleEventStatus(Event& event);
        void ResizeEventArray(Event*& array, size_t& count, size_t newCapacity);
        void ResizeAlarmArray(size_t newCapacity);
        void CopyEvents(const Event* source, Event*& dest, size_t count);
        void AddAlarmNotification(const Event& event, AlarmState state);
        void ResetAllNotificationStates();
        void ResetNotificationState(size_t eventIndex);
    };
}