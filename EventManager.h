#pragma once
#include "Event.h"
#include <vector>

class EventManager {
public:
    // Add an event to the event list
    void addEvent(const Event& event);

    // Delete an event by its index
    void deleteEvent(int eventId);

    // Update an event at the given index
    void updateEvent(int eventId, const Event& event);

    // Get all events
    std::vector<Event> getAllEvents() const;

    // Find events by date
    std::vector<Event> findEventsByDate(const std::tm& date) const;

    // Find events by name
    std::vector<Event> findEventsByName(const std::string& name) const;

    // Sort events by deadline
    void sortEventsByDeadline();

    // Remove events that are in the past
    void removePastEvents();

    // Update events based on their current status
    void updateEvents();

    // Clear all completed events
    void clearAllCompletedEvents();

private:
    std::vector<Event> events;  // Vector to store events
};
