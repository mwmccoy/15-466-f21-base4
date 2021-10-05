#pragma once
#include <string>
#include <vector>
#include <list>
#include <algorithm>

struct Option {
public:
	Option(std::string textIn, std::list<std::string> resourcesIn, std::list<std::string> resourcesOut, std::string next);
	~Option();

	bool active;

	bool setActive(std::vector<std::string>* resources);

	std::string evaluate(std::vector<std::string>* resources);

	std::string text;
	std::string getText(int index);

	std::vector<std::string> resourcesNeeded;
	std::vector<std::string> resourcesGiven;
	std::string nextRoom;
};

class Room {
public:
	Room(std::string toName, std::string textIn);
	~Room();

	std::string name;
	std::string text;
	
	std::vector<Option> options;

	int numActiveOptions(std::vector<std::string> *resources);

	void addOption(Option opt);

	std::string getOption(int index);

	std::string EvaluateOption(int index, std::vector<std::string>* resources);
};


