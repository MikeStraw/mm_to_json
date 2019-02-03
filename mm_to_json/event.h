#pragma once
//#include "entry.h"
#include "meetType.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>


class Event
{
public:
	Event(int number, bool relay, std::string gender, std::string genderDescription, 
		  int minAge, int maxAge, int distance, const std::string &stroke, 
		  const std::string &division, const std::string &round, int eventPtr)
		:ageMax_(maxAge), ageMin_(minAge), distance_(distance), division_(division), 
		 gender_(gender), genderDesc_(genderDescription), number_(number), 
		 relay_(relay), stroke_(stroke), round_(round), eventPtr_(eventPtr)
	{
	}
	~Event()
	{
	}

//	void addEntries(std::vector<Entry *> &entries) { entries_ = entries; }

	// create the event description appropriate for the given meet type
	void createDescription(MeetType meetType);

	int   getDistance() const { return distance_; }
	const std::string getDivision() const { return division_; }
	int   getEventPtr() const { return eventPtr_; }
	const std::string getGender() const { return gender_; }
	int   getNumber() const { return number_; }
	const std::string getRound() const { return round_; }
	const std::string getStroke() const { return stroke_; }
	bool  isRelay() const { return relay_; }

	nlohmann::json toJson() const;

private:
	std::string createAgeGroupDescription(MeetType meetType);
	std::string createGenderDescription();

	int         ageMax_;      // max age for this event
	int         ageMin_;      // miniumum age for this event
	std::string description_; // eg, Boys [age-group] 200 Freestyle - Varsity
	int         distance_;
	std::string division_;    // JV, Varsity, JR, SR, etc
	int         eventPtr_;    // ID of event within Meet Manager
	std::string gender_;      // Gender of participants 'M', 'F', 'C'
	std::string genderDesc_;  // Description of gender 'B'oys, 'G'irls, 'M'en, 'W'omen, 'X'(mixed)
	int         number_;
	bool        relay_;       // true == relay
	std::string round_;       // 'P' == prelim, 'F' == final
	std::string stroke_;

	// TODO:  use unique_ptr<Entry>.
	//        not a big deal since destruction won't happen till end of program anyway.
	//        don't delete in dtor() as the pointers are copied from one Event to another
//	std::vector<Entry *> entries_;  // Need pointer so that polymorphism works
};