#include "mmToJsonConverter.h"
#include "util.h"
#include <iostream>

using namespace std;

// Data struct holding some Entry info
typedef struct {
	int heat;
	int lane;
	std::string seed;
	std::string time;
} EntryHeatLaneTime;




// ****************************************************************************
// ********** Helper Functions
// ****************************************************************************

EntryHeatLaneTime getHeatLaneTime(const string &eventRound, const string &stroke, float seedTime,
	                              int preHeat, int preLane, float preTime,
	                              int finHeat, int finLane, float finTime)
{
	EntryHeatLaneTime entryInfo;

	entryInfo.seed = (seedTime > 0.0 ? numToString(seedTime) : "NT");

	if (eventRound == "P") {
		entryInfo.heat = preHeat;
		entryInfo.lane = preLane;
		entryInfo.time = (preTime > 0.0 ? numToString(preTime) : "");
	}
	else {
		entryInfo.heat = finHeat;
		entryInfo.lane = finLane;
		entryInfo.time = (finTime > 0.0 ? numToString(finTime) : "");
	}

	// For swimming events, the times are seconds.tt.  Convert these to mm:ss.tt
	// For diving events, the times are really the diving scores so leave those alone
	if (stroke != "Diving") {
		entryInfo.seed = timeToMinSec(entryInfo.seed);
		entryInfo.time = timeToMinSec(entryInfo.time);
	}

	return entryInfo;
}

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
			addEntriesToEvent(event);
			session.addEvent(event);
		}

		meet->addSession(session);
	}

	meetJson_ = meet->toJson();
}



// ****************************************************************************
// ********** Private Member Methods
// ****************************************************************************

void mmToJsonConverter::addRelayEntries(Event &event)
{
	string eventIdStr = numToString(event.getEventPtr());
	string sql = "Select Team_no, Team_ltr, Relay_no, Pre_heat, Pre_lane, Fin_heat, Fin_lane, "
		         "ConvSeed_time, Pre_Time, Fin_Time "
		         "From Relay where Event_Ptr = " + eventIdStr;

	nanodbc::result result = nanodbc::execute(dbConn_, sql);
	while (result.next()) {
		int teamNo      = result.get<int>(0);
		string relayLtr = result.get<string>(1, "A");
		int relayNo     = result.get<int>(2);
		int preHeat     = result.get<int>(3, 0);
		int preLane     = result.get<int>(4, 0);
		int finHeat     = result.get<int>(5, 0);
		int finLane     = result.get<int>(6, 0);
		float seedTime  = result.get<float>(7, 0.0);
		float preTime   = result.get<float>(8, 0.0);
		float finTime   = result.get<float>(9, 0.0);

		EntryHeatLaneTime entryInfo = getHeatLaneTime(event.getRound(), event.getStroke(), seedTime,
			                                          preHeat, preLane, preTime, finHeat, finLane, finTime);

		if (entryInfo.heat != 0 && entryInfo.lane != 0) {
			string teamName = getTeamNameByNumber(teamNo);
			vector<Athlete> athletes = getRelayAthletes(event.getEventPtr(), teamNo, relayLtr, event.getRound());

			RelayEntry entry(relayLtr, athletes, entryInfo.heat, entryInfo.lane, teamName, entryInfo.seed, entryInfo.time);
			event.addEntry(entry);
		}
	}
}


void mmToJsonConverter::addIndividualEntries(Event &event)
{
	string eventIdStr = numToString(event.getEventPtr());
	string sql = "Select Ath_no, Pre_heat, Pre_lane, Fin_heat, Fin_lane, ConvSeed_time, Pre_Time, Fin_Time "
		         "From Entry where Event_Ptr = " + eventIdStr;

	nanodbc::result result = nanodbc::execute(dbConn_, sql);
	while (result.next()) {
		int athleteNo  = result.get<int>(0);
		int preHeat    = result.get<int>(1, 0);
		int preLane    = result.get<int>(2, 0);
		int finHeat    = result.get<int>(3, 0);
		int finLane    = result.get<int>(4, 0);
		float seedTime = result.get<float>(5, 0.0);
		float preTime  = result.get<float>(6, 0.0);
		float finTime  = result.get<float>(7, 0.0);

		EntryHeatLaneTime entryInfo = getHeatLaneTime(event.getRound(), event.getStroke(), seedTime,
			                                          preHeat, preLane, preTime, finHeat, finLane, finTime);

		// If there data for this entry, add it to the event
		if (entryInfo.heat != 0 && entryInfo.lane != 0) {
			Athlete athlete = getAthleteByNumber(athleteNo);
			string teamName = athlete.getTeamName();

			IndEntry entry(athlete, entryInfo.heat, entryInfo.lane, teamName, entryInfo.seed, entryInfo.time);
			event.addEntry(entry);	
		}
	}
}

map<int, Athlete> mmToJsonConverter::createAthleteMap()
{
	map<int, Athlete> athleteMap;
	string sql = "Select Ath_no, First_name, Last_name, Team_no, Schl_yr, Ath_age From Athlete";

	nanodbc::result result = nanodbc::execute(dbConn_, sql);

	while (result.next()) {
		int    athlNo   = result.get<int>(0);
		string fname    = result.get<string>(1);
		string lname    = result.get<string>(2);
		int    teamNo   = result.get<int>(3);
		string schlYr   = result.get<string>(4, "");
		int    age      = result.get<int>(5, 0);
		string teamName = getTeamNameByNumber(teamNo);

		Athlete athlete(trim(fname), trim(lname), age, schlYr, teamName);
		athleteMap.insert(make_pair(athlNo, athlete));
	}
	cout << "Returning athlete name map of size: " << athleteMap.size() << endl;
	return athleteMap;
}


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

	nanodbc::result result = nanodbc::execute(dbConn_, sql);

	while (result.next()) {
		int    number = result.get<int>(0);
		string name   = result.get<string>(1);
		name = trim(name);

		if (!name.empty()) {
			divisionMap.insert(make_pair(number, name));
		}
	}
	cout << "Returning division map of size: " << divisionMap.size() << endl;
	return divisionMap;
}


map<int, string> mmToJsonConverter::createTeamNameMap()
{
	map<int, string> teamNameMap;
	string sql = "Select Team_no, Team_abbr, Team_short, Team_lsc From Team";

	nanodbc::result result = nanodbc::execute(dbConn_, sql);

	while (result.next()) {
		int    number    = result.get<int>(0);
		string abbr      = result.get<string>(1, "");
		string shortName = result.get<string>(2, "");
		string lsc       = result.get<string>(2, "");
		string teamName  = (shortName.empty() ? abbr + "-" + lsc : shortName);

		teamNameMap.insert(make_pair(number, trim(teamName)));
	}
	cout << "Returning team name map of size: " << teamNameMap.size() << endl;
	return teamNameMap;
}


vector<Event> mmToJsonConverter::getAllEvents()
{
	vector<Event> events;
	string sql = "Select Event_no, Ind_rel, Event_gender, Event_sex, Low_age, High_age, Event_dist, "
		         "Event_stroke, Div_no, Num_prelanes, Num_finlanes, Event_rounds "
	             "From Event Order by Event_no";
	nanodbc::result result = nanodbc::execute(dbConn_, sql);

	while (result.next()) {
		int number = result.get<int>(0);
		bool relay = (result.get<string>(1) == "R" ? true : false);
		string gender = result.get<string>(2);
		string genderDesc = result.get<string>(3);
		int minAge = result.get<int>(4, 0);
		int maxAge = result.get<int>(5, 0);
		int distance = result.get<int>(6);
		string stroke = getStroke(result.get<string>(7), relay);
		string division = getDivision(result.get<int>(8));
		int preLanes = result.get<int>(9, 0);
		int finLanes = result.get<int>(10, 0);
		int evtRound = result.get<int>(11, 1);

		int numLanes = evtRound == 1 ? preLanes : finLanes;

		// Events from a "meet" rather than a session are always Final and the event pointer
		// is the same as the event number.  
		Event event(number, relay, gender, genderDesc, minAge, maxAge, distance, stroke, division, "F", number, numLanes);
		events.push_back(event);
	}

	return events;
}


Athlete mmToJsonConverter::getAthleteByNumber(int athleteNo)
{

	if (athleteMap_.empty()) {
		athleteMap_ = createAthleteMap();
	}

	return athleteMap_[athleteNo];
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
	nanodbc::result result = nanodbc::execute(dbConn_, sql);

	while (result.next()) {
		int order = result.get<int>(0);
		int sessPtr = result.get<int>(1);
		string round = result.get<string>(2);
		int evtPtr = result.get<int>(3);
		EventRound evtRound(evtPtr, round);

		eventRounds.push_back(evtRound);
	}

	return eventRounds;
}


Event mmToJsonConverter::getEventById(EventRound eventRound)
{
	string eventIdStr = numToString(eventRound.eventPtr_);
	string sql = "Select Event_no, Ind_rel, Event_gender, Event_sex, Low_age, High_age, Event_dist, "
		         "Event_stroke, Div_no, Num_prelanes, Num_finlanes, Event_rounds "
		         "From Event where Event_Ptr = " + eventIdStr;
	nanodbc::result result = nanodbc::execute(dbConn_, sql);

	if (!result.next()) {
		string errStr = "Error in mmToJsonConverter::getEventById, no event found with event_ptr = " + eventIdStr;
		throw exception(errStr.c_str());
	}
	else {
		int number = result.get<int>(0);
		bool relay = (result.get<string>(1) == "R" ? true : false);
		string gender = result.get<string>(2);
		string genderDesc = result.get<string>(3);
		int minAge = result.get<int>(4, 0);
		int maxAge = result.get<int>(5, 0);
		int distance = result.get<int>(6);
		string stroke = getStroke(result.get<string>(7), relay);
		string division = getDivision(result.get<int>(8));
		int preLanes = result.get<int>(9, 0);
		int finLanes = result.get<int>(10, 0);
		int evtRounds = result.get<int>(11, 1);

		int numLanes = evtRounds == 1 ? preLanes : finLanes;
		cout << "getEventById: pre=" << preLanes << ", fin=" << finLanes << ", evtRounds=" << evtRounds << endl;

		Event event(number, relay, gender, genderDesc, minAge, maxAge, distance, 
			        stroke, division, eventRound.round_, eventRound.eventPtr_, numLanes);
		
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


vector<Athlete> mmToJsonConverter::getRelayAthletes(int eventId, int teamNo, const string &teamLtr, const string &round)
{
	vector<Athlete> athletes;
	char sqlBuf[256] = { 0 };

	sprintf_s(sqlBuf, sizeof(sqlBuf),
		      "Select Ath_no from RelayNames where Event_ptr = %d AND team_no = %d AND Team_ltr = '%s' AND Event_round = '%s'",
		      eventId, teamNo, teamLtr.c_str(), round.c_str());
	string sql(sqlBuf);

	nanodbc::result result = nanodbc::execute(dbConn_, sql);
	while (result.next()) {
		int athleteNo = result.get<int>(0);

		athletes.push_back(getAthleteByNumber(athleteNo));
	}

	return athletes;
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


string mmToJsonConverter::getTeamNameByNumber(int teamNo)
{
	if (teamNameMap_.empty()) {
		teamNameMap_ = createTeamNameMap();
	}

	return teamNameMap_[teamNo];
}