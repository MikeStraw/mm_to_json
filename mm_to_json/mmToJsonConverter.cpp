#include "mmToJsonConverter.h"
#include <iostream>


using namespace std;

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
	meet_ = getMeetInfo();
	meetJson_ = meet_->toJson();
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


unique_ptr<Meet> mmToJsonConverter::getMeetInfo()
{
	Meet *meetPtr = nullptr;
	nanodbc::result result = nanodbc::execute(dbConn_, "select Meet_name1, Meet_location, Meet_start, Meet_end, Meet_class, Meet_numlanes from Meet");

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