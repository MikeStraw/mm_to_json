#include "session.h"
#include "util.h"
using namespace std;


Session::Session(int id, int number, const string &name, int day, int startTimeSecs, bool isDefault)
	: default_(isDefault), id_(id), meetDay_(day), name_(name), number_(number)
{
	// convert seconds past midnight to hh:mm
	string ampm = " am";
	int    hour = startTimeSecs / 3600;
	int    min = startTimeSecs % 60;

	if (hour > 11) {
		ampm = " pm";
		hour -= 12;
	}

	string hourStr = numToString(hour);
	string minStr = "00";
	if (min != 0) {
		minStr = numToString(min);
		if (minStr.length() < 2)
			minStr = "0" + minStr;
	}

	startTime_ = hourStr + ":" + minStr + ampm;
}


nlohmann::json Session::toJson() const
{
	nlohmann::json j;
	j["number"] = number_;
	j["name"] = name_;
	j["day"] = meetDay_;
	j["time"] = startTime_;

	nlohmann::json jEvents = nlohmann::json::array();
	for (auto &event : events_) {
		jEvents.push_back(event.toJson());
	}
	j["events"] = jEvents;

	return j;
}
