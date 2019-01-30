#pragma once
#include "meet.h"
#include <nanodbc/nanodbc.h>    // find docs at https://nanodbc.github.io/nanodbc/
#include <nlohmann/json.hpp>


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

	// create the meet from the high-level meet information
	std::unique_ptr<Meet> getMeetInfo();


	nanodbc::connection   dbConn_;    // ODBC connection to the MDB file
	std::unique_ptr<Meet> meet_;      // The meet as a C++ object
	nlohmann::json        meetJson_;  // The meet as a JSON object
};