#include "event.h"
#include "util.h"
#include <sstream>
using namespace std;


nlohmann::json Event::toJson() const
{
	nlohmann::json j;
	j["number"]  = number_;
	j["numLanes"] = numLanes_;
	j["desc"]    = description_;
	j["isRelay"] = isRelay();

	nlohmann::json jEntries = nlohmann::json::array();
	if (isRelay()) {
		for (auto &entry : relayEntries_) {
			jEntries.push_back(entry.toJson());
		}
	}
	else {
		for (auto &entry : individualEntries_) {
			jEntries.push_back(entry.toJson());
		}
	}
	j["entries"] = jEntries;

	return j;
}

void Event::createDescription(MeetType meetType)
{
	// Boys 200 Freestyle - Varsity
	// Girls 200 Medley Relay - Varsity
	// Boys 10 & U 50 Breaststroke
	// TODO:  need more information to know about age-group meets

	string ageGroup = createAgeGroupDescription(meetType);
	string distance = (stroke_ == "Diving" ? "1m" : numToString(distance_));
	string division = (division.empty() ? "" : " - " + division_) ;
	string gender   = createGenderDescription();
	ostringstream os;

	os << gender
	   << ageGroup
	   << distance << " "
	   << stroke_ << " "
	   << (relay_ ? "Relay" : "")
	   << division
		;

	description_ = os.str();
}

// ****************************************************************************
// ********** Private Methods
// ****************************************************************************
string Event::createAgeGroupDescription(MeetType meetType)
{
	string ageGroup;
	
	// Making some assumptions about meet types and whether to return the
	// "9 - 10" or "10 & U" label
	//     * College and High School --> do not use
	//     * Age Group and YMCA      --> do use
	//     * Others                  --> don't know, so return "" 
	if (meetType == MeetType::College || meetType == MeetType::HighSchool) {
		return "";
	}

	if (meetType == MeetType::AgeGroup || meetType == MeetType::YMCA) {

		string lowAge = numToString(ageMin_);
		string highAge = numToString(ageMax_);

		if (ageMin_ > 0) {
			ageGroup = lowAge + " - " + highAge + " ";  // "9 - 10 "
		}
		else {
			ageGroup = highAge + " & U ";               // "10 & U "
		}
	}

	return ageGroup;
}

string Event::createGenderDescription()
{
	static string BOY("Boys ");
	static string GIRL("Girls ");
	static string EMPTY("");
	static string MEN("Mens ");
	static string MIXED("Mixed ");
	static string WOMEN("Womens ");

	if (genderDesc_ == "B")  return BOY;
	if (genderDesc_ == "G")  return GIRL;
	if (genderDesc_ == "M")  return MEN;
	if (genderDesc_ == "W")  return WOMEN;
	if (genderDesc_ == "X")  return MIXED;

	return EMPTY;
}

