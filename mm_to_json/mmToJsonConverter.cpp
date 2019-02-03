#include "mmToJsonConverter.h"
#include "util.h"
#include <iostream>

using namespace std;


// ****************************************************************************
// ********** Helper Functions
// ****************************************************************************

string getStroke(const string &strokeId, bool isRelay)
{
	char id = strokeId.at(0);
	string stroke;

	switch (id)
	{
	case 'A': stroke = "Freestyle";     break;
	case 'B': stroke = "Backstroke";    break;
	case 'C': stroke = "Breaststroke";  break;
	case 'D': stroke = "Butterfly";     break;
	case 'E': {
		if (isRelay)  stroke = "Medley";
		else          stroke = "Individual Medley";
		break;
	}
	case 'F': stroke = "Diving";        break;
	default:  stroke = "";              break;
	}
	return stroke;
}


// ****************************************************************************
// ********** Public Member Methods
// ****************************************************************************

mmToJsonConverter::mmToJsonConverter(const string &mbFileSpec)
{
	dbConn_.connect( createConnectionString(mbFileSpec) );
	if (!dbConn_.connected()) {
		throw exception("Error connecting to the database file.");
	}
	//cout << "mmToJsonConverter::ctor - connected to the db, driver = " << dbConn_.driver_name() << endl;
}


void mmToJsonConverter::convert()
{
	unique_ptr<Meet> meet     = getMeetInfo();
	vector<Session>  sessions = getSessionInfo();

	if (sessions.size() == 0) {
		sessions.push_back(createDefaultSession());
	}

	for (auto &session : sessions) {
		vector<Event> events = getEventsBySession(session);

		for (auto &event : events) {
			event.createDescription(meet->getType());
//			event.addEntries(getEntriesByEvent(event));
			session.addEvent(event);
		}

		meet->addSession(session);
	}

	meetJson_ = meet->toJson();
}



// ****************************************************************************
// ********** Private Member Methods
// ****************************************************************************

string mmToJsonConverter::createConnectionString(const string &mdbFileSpec) const
{
	const string MS_ACCESS_DRIVER = "{Microsoft Access Driver (*.mdb)}";
	const string MM_PASSWORD = "TeP69s)lAd_mW-(J_72u";
	const string MM_USER_ID = "Admin";
	string dbConnString = string("Driver=") + MS_ACCESS_DRIVER + string(";") +
		                  string("Dbq=") + mdbFileSpec + string(";") +
		                  string("Uid=") + MM_USER_ID + string(";") +
		                  string("Pwd=") + MM_PASSWORD;

	return dbConnString;
}


Session mmToJsonConverter::createDefaultSession()
{
	return ( Session(0, 0, "Default Session - All Events", 1, 0, true) );
}


map<int, string> mmToJsonConverter::createDivisionMap()
{
	map<int, string> divisionMap;
	string sql = "Select Div_no, Div_name From Divisions";

	nanodbc::result sessResult = nanodbc::execute(dbConn_, sql);

	while (sessResult.next()) {
		int    number = sessResult.get<int>(0);
		string name = sessResult.get<string>(1);
		name = trim(name);

		if (!name.empty()) {
			divisionMap.insert(make_pair(number, name));
		}
	}
	cout << "Returning division map of size: " << divisionMap.size() << endl;
	return divisionMap;
}


vector<Event> mmToJsonConverter::getAllEvents()
{
	vector<Event> events;
	string sql = "Select Event_no, Ind_rel, Event_gender, Event_sex, Low_age, High_age, Event_dist, Event_stroke, Div_no "
	             "From Event Order by Event_no";
	nanodbc::result sessResult = nanodbc::execute(dbConn_, sql);

	while (sessResult.next()) {
		int number = sessResult.get<int>(0);
		bool relay = (sessResult.get<string>(1) == "R" ? true : false);
		string gender = sessResult.get<string>(2);
		string genderDesc = sessResult.get<string>(3);
		int minAge = sessResult.get<int>(4, 0);
		int maxAge = sessResult.get<int>(5, 0);
		int distance = sessResult.get<int>(6);
		string stroke = getStroke(sessResult.get<string>(7), relay);
		string division = getDivision(sessResult.get<int>(8));

		// Events from a "meet" rather than a session are always Final and the event pointer
		// is the same as the event number
		Event event(number, relay, gender, genderDesc, minAge, maxAge, distance, stroke, division, "F", number);
		events.push_back(event);
	}

	return events;
}


string mmToJsonConverter::getDivision(int divId)
{
	if (divisionMap_.empty()) {
		divisionMap_ = createDivisionMap();
	}

	return divisionMap_[divId];
}


vector<EventRound> mmToJsonConverter::getEventRounds(int sessionPtr)
{
	// BINDING DOES NOT WORK, probably due to MS Access driver.  Fails in the statement.bind();
	vector<EventRound> eventRounds;
	string             sessPtrVal = numToString(sessionPtr);

	string sql = "Select Sess_order, Sess_ptr, Sess_rnd, Event_ptr from Sessitem "
		         "where Sess_ptr =" + sessPtrVal + " order by Sess_order";
	nanodbc::result sessResult = nanodbc::execute(dbConn_, sql);

	while (sessResult.next()) {
		int order = sessResult.get<int>(0);
		int sessPtr = sessResult.get<int>(1);
		string round = sessResult.get<string>(2);
		int evtPtr = sessResult.get<int>(3);
		EventRound evtRound(evtPtr, round);

		eventRounds.push_back(evtRound);
	}

	return eventRounds;
}


Event mmToJsonConverter::getEventById(EventRound eventRound)
{
	string eventIdStr = numToString(eventRound.eventPtr_);
	string sql = "Select Event_no, Ind_rel, Event_gender, Event_sex, Low_age, High_age, Event_dist, Event_stroke, Div_no "
		         "From Event where Event_Ptr = " + eventIdStr;
	nanodbc::result sessResult = nanodbc::execute(dbConn_, sql);

	if (!sessResult.next()) {
		string errStr = "Error in mmToJsonConverter::getEventById, no event found with event_ptr = " + eventIdStr;
		throw exception(errStr.c_str());
	}
	else {
		int number = sessResult.get<int>(0);
		bool relay = (sessResult.get<string>(1) == "R" ? true : false);
		string gender = sessResult.get<string>(2);
		string genderDesc = sessResult.get<string>(3);
		int minAge = sessResult.get<int>(4, 0);
		int maxAge = sessResult.get<int>(5, 0);
		int distance = sessResult.get<int>(6);
		string stroke = getStroke(sessResult.get<string>(7), relay);
		string division = getDivision(sessResult.get<int>(8));

		Event event(number, relay, gender, genderDesc, minAge, maxAge, distance, 
			        stroke, division, eventRound.round_, eventRound.eventPtr_);
		
		return event;
	}
}


vector<Event> mmToJsonConverter::getEventsBySession(const Session &session)
{
	vector<Event> events;

	if (session.isDefault()) {
		events = getAllEvents();
	}
	else {
		vector<EventRound> eventRounds = getEventRounds(session.getId());

		for (auto eventRound : eventRounds) {
			Event event = getEventById(eventRound);
			events.push_back(event);
		}
	}

	return events;
}


unique_ptr<Meet> mmToJsonConverter::getMeetInfo()
{
	Meet  *meetPtr = nullptr;
	string sql     = "select Meet_name1, Meet_location, Meet_start, Meet_end, Meet_class, "
		             "Meet_numlanes from Meet";
	nanodbc::result result = nanodbc::execute(dbConn_, sql);

	if (result.next()) {
		string meetName = result.get<string>(0, "null");
		string location = result.get<string>(1, "null");
		string startDate = result.get<string>(2, "null");
		string endDate = result.get<string>(3, "null");
		int    meetClass = result.get<int>(4, 0);
		int    numLanes = result.get<int>(5, 0);

		//cout << "Creating a meet name: " << meetName << ", date: " << startDate << "\n";
		meetPtr = new Meet(meetName, startDate, meetClass, numLanes);
	}

	return unique_ptr<Meet>(meetPtr);
}


vector<Session> mmToJsonConverter::getSessionInfo()
{
	vector<Session> sessions;
	string          sql = "select Sess_no, Sess_day, Sess_ptr, Sess_starttime, Sess_name "
		                  "from Session order by Sess_no";
	nanodbc::result result = nanodbc::execute(dbConn_, sql);

	while (result.next()) {
		int    sessionNumber = result.get<int>(0);
		int    sessionDay = result.get<int>(1, 1);
		int    sessionPtr = result.get<int>(2);
		int    sessionStart = result.get<int>(3, 32400);     // default 9:00 am
		string sessionName = result.get<string>(4, "null");
		Session session(sessionPtr, sessionNumber, sessionName, sessionDay, sessionStart);

		sessions.push_back(session);
	}

	return sessions;
}