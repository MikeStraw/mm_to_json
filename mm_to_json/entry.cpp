#include "entry.h"
#include <nlohmann/json.hpp>
using namespace std;


nlohmann::json Entry::toJson() const
{
	nlohmann::json j;

	j["heat"] = heat_;
	j["lane"] = lane_;
	j["team"] = team_;
	j["seed"] = seedTime_;
	j["final"] = finalTime_;

	return j;
}

nlohmann::json IndEntry::toJson() const
{
	nlohmann::json j = Entry::toJson();
	j["athlete"] = athlete_.toJson();

	return j;
}

nlohmann::json RelayEntry::toJson() const
{
	nlohmann::json j = Entry::toJson();
	j["relay"]["name"] = name_;

	nlohmann::json jRelayAthletes = nlohmann::json::array();
	for (auto &athlete : athletes_) {
		jRelayAthletes.push_back(athlete.toJson());
	}
	j["relay"]["athletes"] = jRelayAthletes;

	return j;
}