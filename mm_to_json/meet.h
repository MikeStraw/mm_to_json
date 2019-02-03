#pragma once
#include "meetType.h"
#include "session.h"
#include <nlohmann/json.hpp>
#include <string>



class Meet
{
public:
	Meet(const std::string &meetName, const std::string &meetDate, int meetType, int numLanes);

	void addSession(const Session &session) { sessions_.push_back(session); }
	void addSessions(const std::vector<Session> sessions) { sessions_ = sessions; }

	const std::string getDate() const { return date_; }
	const std::string getName() const { return name_; }
	const MeetType    getType() const { return type_; }

	nlohmann::json toJson() const;

private:

	std::string date_;              // date of meet in yyyy-mm-dd format
	std::string name_;              // name of the meet
	int         numLanes_;          // number of lanes in pool
	std::vector<Session> sessions_; // session included in this meet
	MeetType type_;                 // type of meet
};