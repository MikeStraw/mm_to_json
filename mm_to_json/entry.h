#pragma once
#include "athlete.h"
#include <nlohmann/json.hpp>
#include <memory>
#include <string>
#include <vector>


class Entry
{
public:
	Entry(int heat, int lane, const std::string &team,
		const std::string &seedTime, const std::string &finalTime)
		: heat_(heat), finalTime_(finalTime), lane_(lane), seedTime_(seedTime), team_(team)
	{}

	virtual ~Entry() {}
	virtual nlohmann::json toJson() const;

protected:
	int         heat_;
	std::string finalTime_;
	int         lane_;
	std::string seedTime_;
	std::string team_;
};


class IndEntry : public Entry
{
public:
	IndEntry(const Athlete &athlete,
		int heat, int lane, const std::string &team,
		const std::string &seedTime, const std::string &finalTime)
		: Entry(heat, lane, team, seedTime, finalTime), athlete_(athlete)
	{
	}
	~IndEntry() {}

	const Athlete &getAthlete() const { return athlete_; }

	nlohmann::json toJson() const;

private:
	Athlete     athlete_;
};


class RelayEntry : public Entry
{
public:
	RelayEntry(const std::string &name, const std::vector<Athlete> &athletes,
		int heat, int lane, const std::string &team,
		const std::string &seedTime, const std::string &finalTime)
		: Entry(heat, lane, team, seedTime, finalTime), athletes_(athletes), name_(name)
	{}
	~RelayEntry() {}

	nlohmann::json toJson() const;

private:
	std::vector<Athlete> athletes_;
	std::string          name_;
};