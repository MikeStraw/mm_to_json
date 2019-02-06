#pragma once
#include <nlohmann/json.hpp>
#include <string>


class Athlete
{
public:
	Athlete() :age_(0)
	{}

	Athlete(const std::string &first, const std::string &last, int age, const std::string &schoolYear, const std::string &teamName)
		: age_(age), fname_(first), lname_(last), schoolYear_(schoolYear), teamName_(teamName)
	{ }

	const std::string &getTeamName() const { return teamName_; }

	// Return a JSON object representing the Athlete
	nlohmann::json toJson() const
	{
		nlohmann::json j;

		j["age"] = age_;
		j["fname"] = fname_;
		j["lname"] = lname_;
		j["schoolYear"] = schoolYear_;
		j["teamName"] = teamName_;

		return j;
	}

private:
	int         age_;         // may be 0 for HS athlete
	std::string fname_;
	std::string lname_;
	std::string schoolYear_;  // mayb be "" for age-group athlete
	std::string teamName_;
};
