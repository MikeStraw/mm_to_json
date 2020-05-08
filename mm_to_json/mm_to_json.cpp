// mm_to_json.cpp : Convert a Meet Manager DB to json
//
#include "mmToJsonConverter.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>

using namespace std;

typedef struct  {
	string errors;
	bool   help = false;
	string mdbFile;
	string outDir = "./";
	bool   watchFile = false;
} ProcessArgs;

nlohmann::json convertMdbToJson(const string& mdbFileSpec);
bool fileExists(const string &fileSpec);
string getOutFileName(const string& mdbFileSpec);
ProcessArgs processArgs(int argc, char** argv);
int runPgm(const ProcessArgs &pargs);
void usage();
int watchFile(const string& mdbFileSpec, const string& outdir, const nlohmann::json &origJson);
void writeToJsonFile(const nlohmann::json& json, const string& fileSpec);


int main(int argc, char **argv)
{
	int rc = 0;

	ProcessArgs pArgs = processArgs(argc, argv);
	if (pArgs.help) {
		usage();
	}
	else if (!pArgs.errors.empty()) {
		cerr << "Error: " << pArgs.errors << "\n" << endl;
		usage();
		rc = 1;
	}
	else {
		rc = runPgm(pArgs);
	}


    return 0;
}

// ****************************************************************************
// convertMdbToJson
// Convert the MDB file into a JSON object.  Return the JSON object.
// ****************************************************************************
nlohmann::json convertMdbToJson(const string& mdbFileSpec)
{
	mmToJsonConverter converter(mdbFileSpec);
	return converter.convert();
}


// ****************************************************************************
// fileExists
// Return true if the file identified by name exists.
// ****************************************************************************
bool fileExists(const string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}


// ****************************************************************************
// getOutFileName
// Return the output file name based on the basename of the MDB file without
// its extension
// ****************************************************************************
string getOutFileName(const string& mdbFileSpec)
{
	size_t idx = mdbFileSpec.find_last_of("/\\");
	string mdbFileName = idx != string::npos ? mdbFileSpec.substr(idx + 1) : mdbFileSpec;

	// replace the .mdb suffix with .json
	if (mdbFileName.length() > 4) {
		size_t dotIdx = mdbFileName.length() - 4;
		mdbFileName.erase(dotIdx);
	}
	
	return mdbFileName;
}


// ****************************************************************************
// processArgs
// Process the command line args and set the global output directory and
// file watch options
// Return true if the args are valid and the process should continue
// ****************************************************************************
ProcessArgs processArgs(int argc, char** argv)
{
	int mdbFileIdx = argc - 1;
	ProcessArgs pArgs;

	if (argc < 2) {
		pArgs.errors = "MDB file not specified";
		return pArgs;
	}
	if (argc == 2 && string(argv[1]) == "-h") {
		pArgs.help = true;
		return pArgs;
	}

	// process args all the args up to (but not including) the MDB file
	int i = 1;
	for (; i < mdbFileIdx; i++) {
		string arg = string(argv[i]);
		
		if (arg == "-h") {
			pArgs.help = true;
			break;
		}
		else if (arg == "-d") {
			string outDir = string(argv[i + 1]);

			if (outDir.back() != '/' || outDir.back() != '\\') {
				outDir.append("/");
			}
			
			pArgs.outDir = outDir;
			i++;
		}
		else if (arg == "-w") {
			pArgs.watchFile = true;
		}
		else {
			pArgs.errors = "Invalid argument: " + arg;
			break;
		}
	}

	// Grab the MDB file name
	if (pArgs.errors.empty() == true) {		
		if (i == mdbFileIdx) {
			pArgs.mdbFile = argv[mdbFileIdx];
		}
		else {
			pArgs.errors = "MDB file name not specified.  Was output directory forgotten?";
		}
	}

	return pArgs;
}

// ****************************************************************************
// runPgm
// Run the mm_to_json program.  Return the process return code.
// ****************************************************************************
int runPgm(const ProcessArgs &pArgs)
{
	int rc = 0;

	string mdbFileSpec(pArgs.mdbFile);
	if (!fileExists(mdbFileSpec)) {
		cerr << "MDB file [" << mdbFileSpec << "] does not exist." << endl;
		return 2;
	}

	try {
		string jsonFileName = getOutFileName(mdbFileSpec) + ".json";
		nlohmann::json json = convertMdbToJson(mdbFileSpec);

		cout << "Conversion completed, writing to output file " << jsonFileName << endl;
		writeToJsonFile(json, pArgs.outDir + jsonFileName);

		if (pArgs.watchFile) {
			// watchFile does not return ...
			rc = watchFile(mdbFileSpec, pArgs.outDir, json);
		}
	}
	catch (exception ex) {
		cerr << "EXCEPTION: " << ex.what() << endl;
		rc = 3;
	}

	return rc;
}


// ****************************************************************************
// usage
// Print "usage" info.
// ****************************************************************************
void usage()
{
	cerr << "USAGE: mm_to_json [-d out-dir] [-h] [-w] mdb_file\n" 
	        "where: -h     --> produces this help message.\n"
	        "       -d dir --> directory for output (json) file.  Default is ./ \n"
	        "       -w     --> watch the mdb_file and create a new output file\n"
	        "                  if there are detectable changes.\n"
	     << endl;
}


// ****************************************************************************
// watchFile
// Watch the mdbFileSpec file for changes.  After each change, convert the updated
// file to JSON, diff with the previous json and create an update file, if necessary.
// ****************************************************************************
int watchFile(const string& mdbFileSpec, const string& outDir, const nlohmann::json &origJson)
{
	struct stat result;
	int         rc = stat(mdbFileSpec.c_str(), &result);

	if (rc != 0) {
		cerr << "ERROR: watch file " << mdbFileSpec << "stat error " << rc << endl;
		return rc;
	}

	string jsonFileName = getOutFileName(mdbFileSpec);
	auto   prevJson = origJson;
	time_t prevModTime = result.st_mtime;
	int    updateNumber = 1;

	// loop forever, checking for JSON file updates
	do {
		this_thread::sleep_for(chrono::seconds(30));

		rc = stat(mdbFileSpec.c_str(), &result);
		if (rc != 0) {
			cerr << "ERROR: stat returns " << rc << endl;
			break;
		}

		auto curModTime = result.st_mtime;
		if (prevModTime != curModTime) {
			cout << "Watch file has been modified, creating new json  ...\n";

			auto curJson = convertMdbToJson(mdbFileSpec);
			auto jsonDiff = nlohmann::json::diff(prevJson, curJson);

			// jsonDiff is an array of differences.  See if there are any
			cout << "json diff array size = " <<jsonDiff.size() << endl;
			if (jsonDiff.size() > 0) {
				cout << "diff: " << jsonDiff << "\n\n";

				char updateFileSpec[512];
				sprintf(updateFileSpec, "%s%s_%03d.json", outDir.c_str(), jsonFileName.c_str(), updateNumber);
				writeToJsonFile(curJson, updateFileSpec);
				updateNumber++;
			}

			prevJson = curJson;
			prevModTime = curModTime;

		}
	} while (rc == 0);

	return rc;

}


// ****************************************************************************
// writeToJsonFile
// Write the json object to the file identified by fileSpec.
// ****************************************************************************
void writeToJsonFile(const nlohmann::json& json, const string& fileSpec)
{
	ofstream jsonFs(fileSpec);
	jsonFs << mmToJsonConverter::toJsonStr(json) << endl;
}