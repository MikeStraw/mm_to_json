#pragma once
#include <string>

enum class MeetType { AgeGroup, SeniorOpen, HighSchool, College, YMCA, Masters, Disabled, Unknown };

inline MeetType meetTypeFromInt(int meetTypeVal)
{
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

inline std::string meetTypeToString(MeetType type)
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