#pragma once
#include <string>

// DateTime structure for representing date and time
struct DateTime 
{
    int year, month, day, hour, minute;
};

class Utils 
{
public:
    // Function to format DateTime as "DD/MM/YYYY HH:mm"
    static std::string formatDateTime(const DateTime& dt);

    // Function to compare two DateTime objects
    static bool isDateTimeBefore(const DateTime& t1, const DateTime& t2);

    // Get the current DateTime as a DateTime structure
    static void getCurrentDateTime(DateTime& currentDateTime);
};


