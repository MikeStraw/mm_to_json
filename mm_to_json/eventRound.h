#pragma once
#include <string>

// struct for holding event round (prelim/final) information
class EventRound {
public:
	EventRound(int eventPtr, const std::string &round) : eventPtr_(eventPtr), round_(round)
	{ }
	 
	int         eventPtr_;
	std::string round_;     // "P" - prelim, "F" - final
};