// mm_to_json.cpp : Convert a Meet Manager DB to json
//
#include "mmToJsonConverter.h"
#include <iostream>

bool fileExists(const std::string &fileSpec);
void usage();


int main(int argc, char **argv)
{
	if (argc < 2) {
		usage();
		return 1;
	}

	std::string mdbFileSpec(argv[1]);
	if (!fileExists(mdbFileSpec)) {
		std::cerr << "MDB file [" << mdbFileSpec << "] does not exist." << std::endl;
		return 1;
	}

	try {
		mmToJsonConverter converter(mdbFileSpec);
		converter.convert();
		std::cout << "Conversion complete, meet: \n" << converter.toJsonStr() << std::endl;
	}
	catch (std::exception ex) {
		std::cerr << "EXCEPTION: " << ex.what() << std::endl;
	}

    return 0;
}


// ****************************************************************************
// fileExists
// Return true if the file identified by name exists.
// ****************************************************************************
bool fileExists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}


// ****************************************************************************
// usage
// Print "usage" info.
// ****************************************************************************
void usage()
{
	std::cerr << "USAGE: mm_to_json mdb_file" << std::endl;
}

