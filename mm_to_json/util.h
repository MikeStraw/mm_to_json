#pragma once
#include <string>


inline std::string numToString(int num)
{
	char buf[64];
	sprintf_s(buf, sizeof(buf) - 1, "%d", num);

	return std::string(buf);
}


inline std::string trim(const std::string& str)
{
	size_t first = str.find_first_not_of(' ');
	if (std::string::npos == first)
	{
		return str;
	}
	size_t last = str.find_last_not_of(' ');
	return str.substr(first, (last - first + 1));
}