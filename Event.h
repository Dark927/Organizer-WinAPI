#pragma once
#include <string>
#include "Utils.h" 

class Event {
public:
    // Constructor
    Event(std::string name, std::string description, std::string notes, DateTime creationDate, DateTime deadline,
        int importance, bool cyclic, int repeatInterval, bool completed);

    Event();


    // Methods for event manipulation
    std::string getName() const;
    std::string getDescription() const;
    std::string getNotes() const;
    std::string getStatus() const;
    DateTime getDeadline() const;

    // Print event details
    void printEventDetails() const;

    // Toggle event status
    void toggleStatus(const std::string& newStatus);

    // Copy the event
    Event copyEvent() const;

    // Check if event is overdue
    bool isOverdue() const;

private:
    std::string name;
    std::string description;
    std::string notes;
    DateTime creationDate;
    DateTime deadline;
    int importance;
    bool cyclic;
    int repeatInterval;
    std::string status;

    // Format date and time using the Utils class
    std::string formatDateTime(const DateTime& dt) const;

    // Compare two DateTimes using the Utils class
    bool isDateTimeBefore(const DateTime& t1, const DateTime& t2) const;
};
