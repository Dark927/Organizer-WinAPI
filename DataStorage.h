#pragma once

#include <vector>
#include "Event.h"

class DataStorage {
public:
    DataStorage();
    ~DataStorage();
    void LoadData();
    void SaveData();
    void AddEvent(const Event& event);
    std::vector<Event> GetAllEvents() const;

private:
    std::vector<Event> events;
};
