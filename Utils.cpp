#include "Utils.h"
#include <sstream>
#include <ctime>
#include <algorithm>

std::string Utils::formatDateTime(const DateTime& dt)
{
	std::ostringstream oss;
	oss << dt.day << "/" << dt.month << "/" << dt.year << " " << dt.hour << ":" << (dt.minute < 10 ? "0" : "") << dt.minute;
	return oss.str();
}

bool Utils::isDateTimeBefore(const DateTime& t1, const DateTime& t2)
{
	if (t1.year < t2.year) return true;
	if (t1.year > t2.year) return false;
	if (t1.month < t2.month) return true;
	if (t1.month > t2.month) return false;
	if (t1.day < t2.day) return true;
	if (t1.day > t2.day) return false;
	if (t1.hour < t2.hour) return true;
	if (t1.hour > t2.hour) return false;
	if (t1.minute < t2.minute) return true;
	return false;
}

void Utils::getCurrentDateTime(DateTime& currentDateTime)
{
	time_t now = time(0);
	struct tm currentTime;
	localtime_s(&currentTime, &now);
	currentDateTime.day = currentTime.tm_mday;
	currentDateTime.month = currentTime.tm_mon + 1;  // tm_mon is 0-based
	currentDateTime.year = currentTime.tm_year + 1900;  // tm_year is years since 1900
	currentDateTime.hour = currentTime.tm_hour;
	currentDateTime.minute = currentTime.tm_min;
}

// Helper function to convert a string to lowercase
std::string Utils::ToLowerCase(const std::string& input)
{
	std::string result = input;
	std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });
	return result;
}