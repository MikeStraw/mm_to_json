#pragma once
#include "eventRound.h"
#include "meet.h"
#include <nanodbc/nanodbc.h>    // find docs at https://nanodbc.github.io/nanodbc/
#include <nlohmann/json.hpp>
#include <map>
#include <vector>


class mmToJsonConverter
{
public:
	// Create a converter for the Meet Manager database file identified by mbFileSpec.
	// Throws an xxx exception if the database file cannot be opened.
	mmToJsonConverter(const std::string &mbFileSpec);

	// convert - Convert the meet information into a JSON representation.
	//           Throws an xxx exception if the conversion fails.
	void convert();

	// toJsonStr - return the converted meet information as a JSON string
	std::string toJsonStr() const { return meetJson_.dump(4);  }

private:
	// create the MS Access connection string to this file
	std::string createConnectionString(const std::string &mdbFileSpec) const;

	// create a default session using all the events from the meet
	Session createDefaultSession();

	// create the map of division IDs to division names
	std::map<int, std::string> createDivisionMap();

	// get all the events for a meet
	std::vector<Event> getAllEvents();

	// get the division name associated with the division id
	std::string getDivision(int divId);

	// get a single event by its ID and Prelim/Final indicator
	Event getEventById(EventRound eventRound);

	// get the events associated with a session
	std::vector<Event> getEventsBySession(const Session &session);

	std::vector<EventRound> getEventRounds(int sessionPtr);

	// get the high-level meet information from the DB
	std::unique_ptr<Meet> getMeetInfo();

	// get the high-level session information from the DB
	std::vector<Session> getSessionInfo();


	nanodbc::connection        dbConn_;       // ODBC connection to the MDB file
	std::map<int, std::string> divisionMap_;  // cache of division ID to division name
	nlohmann::json             meetJson_;     // The meet as a JSON object
};