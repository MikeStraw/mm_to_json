#include "meet.h"
//#include "util.h"
#include <iostream>

using namespace std;

// Define the 2 helper function to convert from int and to string
MeetType meetTypeFromInt(int meetTypeVal)
{
	//enum class MeetType { AgeGroup, SeniorOpen, HighSchool, College, YMCA, Masters, Disabled, Unknown};
	switch (meetTypeVal) {
		case 1:  return  MeetType::AgeGroup;
		case 2:  return  MeetType::SeniorOpen;
		case 3:  return  MeetType::HighSchool;
		case 4:  return  MeetType::College;
		case 5:  return  MeetType::YMCA;
		case 6:  return  MeetType::Masters;
		case 7:  return  MeetType::Disabled;
		default: return  MeetType::Unknown;
	};
}

string meetTypeToString(MeetType type)
{
	switch (type) {
		case MeetType::AgeGroup:   return "AgeGroup";
		case MeetType::SeniorOpen: return "SeniorOpen";
		case MeetType::HighSchool: return "HighSchool";
		case MeetType::College:    return "College";
		case MeetType::YMCA:       return "YMCA";
		case MeetType::Masters:    return "Masters";
		case MeetType::Disabled:   return "Disabled";
		default:                   return "Unkown";
	};
}


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
//	for (auto &session : sessions_) {
//		jSessions.push_back(session.toJson());
//	}
	j["sessions"] = jSessions;

	return j;
}

