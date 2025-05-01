#pragma once

#include <string>
#include <ctime>

class Alarm {
public:
    Alarm();
    void SetAlarm(const std::tm& time, const std::string& message);
    void StartAlarm();
    void StopAlarm();

private:
    std::tm alarmTime;
    std::string alarmMessage;
    bool isActive;
};
