#include "meet.h"
#include <iostream>

using namespace std;


Meet::Meet(const string &meetName, const string &meetDate, int meetType, int numLanes)
	: name_(meetName), numLanes_(numLanes)
{
	// meetDate is of the form:  yyyy-mm-dd hh-mi-ss 
	// we just want the date portion.
	date_ = meetDate.substr(0, meetDate.find(" "));
	type_ = meetTypeFromInt(meetType);
}



nlohmann::json Meet::toJson() const
{
	nlohmann::json j;
	j["date"] = date_;
	j["name"] = name_;
	j["numLanes"] = numLanes_;
	j["type"] = meetTypeToString(type_);

	nlohmann::json jSessions = nlohmann::json::array();
	for (auto &session : sessions_) {
		jSessions.push_back(session.toJson());
	}
	j["sessions"] = jSessions;

	return j;
}