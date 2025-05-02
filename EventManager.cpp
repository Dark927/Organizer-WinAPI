#include "EventManager.h"
#include <algorithm>
#include <ctime>

void EventManager::addEvent(const Event& event)
{
	events.push_back(event);
}

void EventManager::deleteEvent(int eventId)
{
	if (eventId >= 0 && eventId < events.size())
	{
		events.erase(events.begin() + eventId);
	}
}

void EventManager::updateEvent(int eventId, const Event& event)
{
	if (eventId >= 0 && eventId < events.size())
	{
		events[eventId] = event;
	}
}

std::vector<Event> EventManager::getAllEvents() const
{
	return events;
}

std::vector<Event> EventManager::findEventsByDate(const std::tm& date) const
{
	std::vector<Event> result;
	for (const auto& event : events)
	{
		if (event.getDeadline().year == date.tm_year + 1900 &&
			event.getDeadline().month == date.tm_mon + 1 &&
			event.getDeadline().day == date.tm_mday)
		{
			result.push_back(event);
		}
	}
	return result;
}

std::vector<Event> EventManager::findEventsByName(const std::string& name) const
{
	std::vector<Event> result;
	for (const auto& event : events)
	{
		if (event.getName() == name)
		{
			result.push_back(event);
		}
	}
	return result;
}

void EventManager::sortEventsByDeadline()
{
	std::sort(events.begin(), events.end(), [](const Event& a, const Event& b)
		{
			return a.getDeadline().year < b.getDeadline().year ||
				(a.getDeadline().year == b.getDeadline().year &&
					(a.getDeadline().month < b.getDeadline().month ||
						(a.getDeadline().month == b.getDeadline().month &&
							a.getDeadline().day < b.getDeadline().day)));
		});
}

void EventManager::removePastEvents()
{
	time_t now = time(0);
	struct tm currentTime;
	localtime_s(&currentTime, &now);
	std::tm currentDate = currentTime;

	events.erase(
		std::remove_if(events.begin(), events.end(),
			[&currentDate](const Event& event)
			{
				return event.getDeadline().year < currentDate.tm_year + 1900 ||
					(event.getDeadline().year == currentDate.tm_year + 1900 &&
						event.getDeadline().month < currentDate.tm_mon + 1) ||
					(event.getDeadline().year == currentDate.tm_year + 1900 &&
						event.getDeadline().month == currentDate.tm_mon + 1 &&
						event.getDeadline().day < currentDate.tm_mday);
			}),
		events.end());
}

void EventManager::updateEvents()
{
	for (auto& event : events)
	{
		// Check for overdue status or completion here
		if (event.getStatus() != "Completed" && event.isOverdue())
		{
			event.toggleStatus("Overdue");
		}
	}
}

void EventManager::clearAllCompletedEvents()
{
	events.erase(
		std::remove_if(events.begin(), events.end(),
			[](const Event& event)
			{
				return event.getStatus() == "Completed";
			}),
		events.end());
}
