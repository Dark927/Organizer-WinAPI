#include "Event.h"
#include <iostream>

Event::Event(std::string name, std::string description, std::string notes, DateTime creationDate, DateTime deadline,
    int importance, bool cyclic, int repeatInterval, bool completed)
    : name(name), description(description), notes(notes), creationDate(creationDate), deadline(deadline),
    importance(importance), cyclic(cyclic), repeatInterval(repeatInterval), status(completed ? "Completed" : "Not Completed") {}

Event::Event() {}

std::string Event::getName() const {
    return name;
}

std::string Event::getDescription() const {
    return description;
}

std::string Event::getNotes() const {
    return notes;
}

std::string Event::getStatus() const {
    return status;
}

DateTime Event::getDeadline() const {
    return deadline;
}

void Event::printEventDetails() const {
    std::cout << "Event: " << name << std::endl;
    std::cout << "Description: " << description << std::endl;
    std::cout << "Notes: " << notes << std::endl;
    std::cout << "Created on: " << formatDateTime(creationDate) << std::endl;
    std::cout << "Deadline: " << formatDateTime(deadline) << std::endl;
    std::cout << "Importance: " << importance << " stars" << std::endl;
    std::cout << "Status: " << status << std::endl;
    std::cout << "Cyclic: " << (cyclic ? "Yes" : "No") << std::endl;
    std::cout << "Repeat interval: " << repeatInterval << " days" << std::endl;
}

void Event::toggleStatus(const std::string& newStatus) {
    if (newStatus == "Completed" || newStatus == "Not Completed" || newStatus == "Overdue") {
        status = newStatus;
    }
    else {
        std::cout << "Invalid status!" << std::endl;
    }
}

Event Event::copyEvent() const {
    return Event(name, description, notes, creationDate, deadline, importance, cyclic, repeatInterval, status == "Completed");
}

bool Event::isOverdue() const {
    DateTime currentDateTime;
    Utils::getCurrentDateTime(currentDateTime);
    return isDateTimeBefore(deadline, currentDateTime);
}

bool Event::isDateTimeBefore(const DateTime& t1, const DateTime& t2) const {
    return Utils::isDateTimeBefore(t1, t2);
}

std::string Event::formatDateTime(const DateTime& dt) const {
    return Utils::formatDateTime(dt);
}
