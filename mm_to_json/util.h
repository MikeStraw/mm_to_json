#pragma once
#include <string>


inline std::string numToString(int num)
{
	char buf[64];
	sprintf_s(buf, sizeof(buf) - 1, "%d", num);

	return std::string(buf);
}

inline std::string numToString(float num)
{
	char buf[64];
	sprintf_s(buf, sizeof(buf) - 1, "%.2f", num);

	return std::string(buf);
}


// convert time of the form ss.tt into mi:ss.tt
inline std::string timeToMinSec(const std::string &inTimeStr)
{
	std::string outStr;

	if (inTimeStr.empty() || inTimeStr == "0.0" || inTimeStr == "NT") {
		return inTimeStr;
	}
	// check for format of ss.tt
	size_t period = inTimeStr.find('.', 0);
	if (period == std::string::npos || inTimeStr.length() + 1 <= period) {
		return inTimeStr;
	}

	std::string ss = inTimeStr.substr(0, period);
	std::string tt = inTimeStr.substr(period + 1);
	int ssInt = atoi(ss.c_str());
	int miInt = ssInt / 60;
	ssInt = ssInt % 60;

	if (miInt > 0) {
		char numBuf[32];
		sprintf_s(numBuf, sizeof(numBuf), "%d:%02d.%s", miInt, ssInt, tt.c_str());
		outStr = std::string(numBuf);
	}
	else {
		outStr = inTimeStr;
	}

	return outStr;
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

