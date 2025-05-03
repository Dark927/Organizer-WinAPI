#pragma once
#include <string>
#include "Windows.h"

class Utils
{
public:
	static std::string ToLowerCase(const std::string& input);

	static int CompareSystemTimes(const SYSTEMTIME& a, const SYSTEMTIME& b);
};


