#define UNICODE
#define _UNICODE

#include "EventManager.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include "UIHelpers.h"
#include "Utils.h"

namespace EventsManagerControl
{
	EventsManager::EventsManager() :
		savedEvents(nullptr),
		currentEvents(nullptr),
		savedEventCount(0),
		currentEventCount(0),
		eventCapacity(0),
		dataFilePath(),
		activeAlarms(nullptr),
		activeAlarmCount(0),
		activeAlarmCapacity(0)
	{
		Initialize();
	}

	EventsManager::~EventsManager()
	{
		delete[] savedEvents;
		delete[] currentEvents;
	}

	void EventsManager::Initialize()
	{
		// Set data file path next to executable
		wchar_t exePath[MAX_PATH];
		GetModuleFileNameW(NULL, exePath, MAX_PATH);

		// Find last backslash
		wchar_t* lastSlash = wcsrchr(exePath, L'\\');
		if (lastSlash)
		{
			size_t pathLen = lastSlash - exePath + 1;
			dataFilePath = std::wstring(exePath, pathLen) + L"events.txt";
		}
		else
		{
			dataFilePath = L"events.txt";
		}

		// Load existing events
		LoadFromFile(dataFilePath);
	}

	void EventsManager::ResizeEventArray(Event*& array, size_t& count, size_t newCapacity)
	{
		Event* newEvents = new Event[newCapacity];

		// Copy existing events
		for (size_t i = 0; i < count; i++)
		{
			newEvents[i] = array[i];
		}

		delete[] array;
		array = newEvents;
		eventCapacity = newCapacity;
	}

	void EventsManager::CopyEvents(const Event* source, Event*& dest, size_t count)
	{
		delete[] dest;
		dest = new Event[count];
		for (size_t i = 0; i < count; i++)
		{
			dest[i] = source[i];
		}
	}

	void EventsManager::AddEvent(const Event& event)
	{
		if (currentEventCount >= eventCapacity)
		{
			size_t newCapacity = eventCapacity == 0 ? 4 : eventCapacity * 2;
			ResizeEventArray(currentEvents, currentEventCount, newCapacity);
		}

		currentEvents[currentEventCount] = event;
		UpdateSingleEventStatus(currentEvents[currentEventCount]);
		currentEventCount++;
	}

	void EventsManager::DeleteEvent(size_t index)
	{
		if (index < currentEventCount)
		{
			for (size_t i = index; i < currentEventCount - 1; i++)
			{
				currentEvents[i] = currentEvents[i + 1];
			}
			currentEventCount--;
		}
	}

	void EventsManager::ClearPastDueEvents()
	{
		size_t writeIndex = 0;
		for (size_t readIndex = 0; readIndex < currentEventCount; readIndex++)
		{
			if (!currentEvents[readIndex].isPastDue)
			{
				if (writeIndex != readIndex)
				{
					currentEvents[writeIndex] = currentEvents[readIndex];
				}
				writeIndex++;
			}
		}
		currentEventCount = writeIndex;
	}

	void EventsManager::UpdateEventsStatus()
	{
		for (size_t i = 0; i < currentEventCount; i++)
		{
			UpdateSingleEventStatus(currentEvents[i]);
		}
	}

	void EventsManager::ProcessRecurringEvents()
	{
		for (size_t i = 0; i < currentEventCount; i++)
		{
			Event* event = &currentEvents[i];
			if (event->isRecurring && event->isPastDue && !event->recurringWasCreated)
			{
				// Calculate next occurrence
				SYSTEMTIME nextDate = event->targetDateTime;

				// Add recurrence interval days
				nextDate = Utils::AddMinutesToSystemTime(nextDate, event->recurrenceInterval * 1440);  // 1440 minutes = 1 day

				if (!Utils::IsDateTimeInPast(nextDate)) // next occurrence is in the future
				{
					event->targetDateTime = nextDate;
				}
			}
		}
	}

	void EventsManager::CommitChanges()
	{
		CopyEvents(currentEvents, savedEvents, currentEventCount);
		savedEventCount = currentEventCount;
	}

	void EventsManager::DiscardChanges()
	{
		CopyEvents(savedEvents, currentEvents, savedEventCount);
		currentEventCount = savedEventCount;
		eventCapacity = currentEventCount;
		CheckForAlarms();
	}

	bool EventsManager::LoadFromFile(const std::wstring& filePath)
	{
		std::wifstream file(filePath.c_str(), std::ios::binary);
		if (!file) return false;

		// Clear existing events
		delete[] savedEvents;
		delete[] currentEvents;
		savedEvents = NULL;
		currentEvents = NULL;
		savedEventCount = 0;
		currentEventCount = 0;
		eventCapacity = 0;

		dataFilePath = filePath;

		std::wstring line;
		while (std::getline(file, line))
		{
			if (line.empty()) continue;

			if (currentEventCount >= eventCapacity)
			{
				size_t newCapacity = eventCapacity == 0 ? 4 : eventCapacity * 2;
				ResizeEventArray(currentEvents, currentEventCount, newCapacity);
			}

			Event* event = &currentEvents[currentEventCount];
			std::wistringstream iss(line);
			wchar_t sep;

			// Read fields (same as before)
			std::getline(iss, event->name, L'|');
			std::getline(iss, event->description, L'|');
			std::getline(iss, event->notes, L'|');
			iss >> event->importance >> sep;
			iss >> event->hasAlarm >> sep;
			iss >> event->isRecurring >> sep;
			iss >> event->recurrenceInterval >> sep;
			iss >> event->isPastDue >> sep;

			// Read dates (same as before)
			iss >> event->creationDate.wYear >> sep;
			iss >> event->creationDate.wMonth >> sep;
			iss >> event->creationDate.wDay >> sep;
			iss >> event->creationDate.wHour >> sep;
			iss >> event->creationDate.wMinute >> sep;

			iss >> event->targetDateTime.wYear >> sep;
			iss >> event->targetDateTime.wMonth >> sep;
			iss >> event->targetDateTime.wDay >> sep;
			iss >> event->targetDateTime.wHour >> sep;
			iss >> event->targetDateTime.wMinute;
			event->targetDateTime.wSecond = 0;
			event->targetDateTime.wMilliseconds = 0;
			event->targetDateTime.wDayOfWeek = 0;

			currentEventCount++;
		}

		// Initialize saved events to match current events
		CommitChanges();

		return true;
	}

	bool EventsManager::SaveToCurrentFile()
	{
		return SaveToFile(dataFilePath);
	}

	bool EventsManager::SaveToFile(const std::wstring& filePath)
	{
		std::wofstream file(filePath.c_str(), std::ios::binary);
		if (!file) return false;

		for (size_t i = 0; i < currentEventCount; i++)
		{
			const Event* event = &currentEvents[i];
			file << event->name << L"|"
				<< event->description << L"|"
				<< event->notes << L"|"
				<< event->importance << L"|"
				<< event->hasAlarm << L"|"
				<< event->isRecurring << L"|"
				<< event->recurrenceInterval << L"|"
				<< event->isPastDue << L"|"
				<< event->creationDate.wYear << L"|"
				<< event->creationDate.wMonth << L"|"
				<< event->creationDate.wDay << L"|"
				<< event->creationDate.wHour << L"|"
				<< event->creationDate.wMinute << L"|"
				<< event->targetDateTime.wYear << L"|"
				<< event->targetDateTime.wMonth << L"|"
				<< event->targetDateTime.wDay << L"|"
				<< event->targetDateTime.wHour << L"|"
				<< event->targetDateTime.wMinute << L"\n";
		}

		// Update saved events to match current state
		CommitChanges();
		dataFilePath = filePath;

		return true;
	}

	void EventsManager::UpdateEvent(size_t index, const Event& updatedEvent)
	{
		if (index < currentEventCount)
		{
			// Preserve notification state if the time didn't change
			if (memcmp(&currentEvents[index].targetDateTime,
				&updatedEvent.targetDateTime,
				sizeof(SYSTEMTIME)) != 0)
			{
				// Time changed - reset notification state
				currentEvents[index].notificationState = NotificationState::NotNotified;
			}

			currentEvents[index] = updatedEvent;
		}
	}

	const Event* EventsManager::GetCurrentEvents() const
	{
		return currentEvents;
	}

	const Event* EventsManager::GetSavedEvents() const
	{
		return savedEvents;
	}

	size_t EventsManager::GetCurrentEventCount() const
	{
		return currentEventCount;
	}

	size_t EventsManager::GetSavedEventCount() const
	{
		return currentEventCount;
	}

	void EventsManager::UpdateSingleEventStatus(Event& event)
	{
		event.isPastDue = Utils::IsDateTimeInPast(event.targetDateTime);
	}

	void EventsManager::SortEvents(SortCriteria criteria, bool ascending)
	{
		int result = 0;

		// Bubble sort implementation
		for (size_t i = 0; i < currentEventCount; ++i)
		{
			for (size_t j = i + 1; j < currentEventCount; ++j)
			{
				bool swap = false;

				switch (criteria)
				{
				case Name:
					result = currentEvents[i].name.compare(currentEvents[j].name);
					break;

				case Deadline:
					result = Utils::CompareSystemTimes(currentEvents[i].targetDateTime, currentEvents[j].targetDateTime);
					break;

				case Importance:
					result = currentEvents[j].importance - currentEvents[i].importance;
					break;

				case CreationDate:
					result = Utils::CompareSystemTimes(currentEvents[j].creationDate, currentEvents[i].creationDate);
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
					Event temp = currentEvents[i];
					currentEvents[i] = currentEvents[j];
					currentEvents[j] = temp;
				}
			}
		}
	}

	void EventsManager::ResizeAlarmArray(size_t newCapacity)
	{
		AlarmNotification* newAlarms = new AlarmNotification[newCapacity];

		for (size_t i = 0; i < activeAlarmCount; i++)
		{
			newAlarms[i] = activeAlarms[i];
		}

		delete[] activeAlarms;
		activeAlarms = newAlarms;
		activeAlarmCapacity = newCapacity;
	}

	void EventsManager::AddAlarmNotification(const Event& event, AlarmState state)
	{
		if (activeAlarmCount >= activeAlarmCapacity)
		{
			size_t newCapacity = activeAlarmCapacity == 0 ? 4 : activeAlarmCapacity * 2;
			ResizeAlarmArray(newCapacity);
		}

		activeAlarms[activeAlarmCount] = { &event, state };
		activeAlarmCount++;
	}

	void EventsManager::CheckForAlarms()
	{
		activeAlarmCount = 0;

		SYSTEMTIME now;
		GetLocalTime(&now);

		for (size_t i = 0; i < currentEventCount; i++)
		{
			Event& event = currentEvents[i];
			if (!event.hasAlarm || event.isPastDue) continue;

			bool isPastDue = Utils::IsDateTimeInPast(event.targetDateTime);

			if (isPastDue)
			{
				return;
			}
			
			int minutesDiff = Utils::CalculateMinutesDifference(now, event.targetDateTime);

			if (minutesDiff <= 3 && event.notificationState != NotificationState::ThreeMinutesNotified)
			{
				AddAlarmNotification(event, AlarmState::ThreeMinutesBefore);
				event.notificationState = NotificationState::ThreeMinutesNotified;
			}
			else if (minutesDiff <= 15 && event.notificationState < NotificationState::FifteenMinutesNotified)
			{
				AddAlarmNotification(event, AlarmState::FifteenMinutesBefore);
				event.notificationState = NotificationState::FifteenMinutesNotified;
			}
			else if (minutesDiff <= 30 && event.notificationState < NotificationState::ThirtyMinutesNotified)
			{
				AddAlarmNotification(event, AlarmState::ThirtyMinutesBefore);
				event.notificationState = NotificationState::ThirtyMinutesNotified;
			}
			else if (minutesDiff <= 60 && event.notificationState < NotificationState::OneHourNotified)
			{
				AddAlarmNotification(event, AlarmState::OneHourBefore);
				event.notificationState = NotificationState::OneHourNotified;
			}
		}
	}

	void EventsManager::ResetAllNotificationStates()
	{
		for (size_t i = 0; i < currentEventCount; i++)
		{
			currentEvents[i].notificationState = NotificationState::NotNotified;
		}
	}

	void EventsManager::ResetNotificationState(size_t eventIndex)
	{
		if (eventIndex < currentEventCount)
		{
			currentEvents[eventIndex].notificationState = NotificationState::NotNotified;
		}
	}

	const AlarmNotification* EventsManager::GetActiveAlarms() const
	{
		return activeAlarms;
	}

	size_t EventsManager::GetActiveAlarmCount() const
	{
		return activeAlarmCount;
	}

	void EventsManager::ClearAlarmNotifications()
	{
		activeAlarmCount = 0;
	}
}