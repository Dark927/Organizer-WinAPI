#include "Utils.h"
#include <sstream>
#include <ctime>
#include <algorithm>

// Helper function to convert a string to lowercase
std::string Utils::ToLowerCase(const std::string& input)
{
	std::string result = input;
	std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });
	return result;
}


int Utils::CompareSystemTimes(const SYSTEMTIME& a, const SYSTEMTIME& b)
{
	FILETIME ftA, ftB;
	SystemTimeToFileTime(&a, &ftA);
	SystemTimeToFileTime(&b, &ftB);
	return CompareFileTime(&ftA, &ftB);
}