#pragma once
#include "event.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

class Session
{
public:
	Session(int id, int number, const std::string &name, int day, int startTimeSecs, bool isDefault = false);

	void addEvent(const Event &event)   { events_.push_back(event); }

	int   getId() const { return id_; }
	int   geMeetDay() const { return meetDay_; }
	const std::string getName() const { return name_; }
	int   getNumber() const { return number_; }
	const std::string getStartTime() const { return startTime_; }

	bool isDefault() const { return default_; }

	nlohmann::json toJson() const;

private:
	bool               default_;       // true if this is a default (ie virtual) session
	std::vector<Event> events_;        // Events included in this session
	int                id_;            // handle to this session (sess_ptr)
	int                meetDay_;       // 1 == day of meet, 2 == next day, ...
	std::string        name_;          // name of this session
	int                number_;        // session number
	std::string        startTime_;     // hh:mm am/pm
};
