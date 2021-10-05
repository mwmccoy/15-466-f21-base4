#include "RoomCode.hpp"

Room::Room(std::string toName, std::string textIn)
{
	name = toName;
	text = textIn;
	options = std::vector<Option>();
}

Room::~Room()
{

}

int Room::numActiveOptions(std::vector<std::string> *resources)
{
	int size = 0;
	for (size_t i = 0; i < options.size(); i++)
	{
		if (options[i].setActive(resources))
		{
			size++;
		}
	}
	return size;
}

void Room::addOption(Option opt)
{
	options.push_back(opt);
}

std::string Room::getOption(int index)
{
	//Doing this the stupid, slow way that always works.
	int checkIndex = 0;
	for (size_t i = 0; i < options.size(); i++)
	{
		if (options[i].active)
		{
			if (index == checkIndex)
			{
				return options[i].getText(checkIndex);
			}
			checkIndex++;
		}
	}
	return "No option found! This is a bug.";
}

std::string Room::EvaluateOption(int index, std::vector<std::string>* resources)
{
	//Doing this the stupid, slow way that always works.
	int checkIndex = 0;
	for (size_t i = 0; i < options.size(); i++)
	{
		if (options[i].active)
		{
			if (index == checkIndex)
			{
				return options[i].evaluate(resources);
			}
			checkIndex++;
		}
	}
	return "No option found! This is a bug.";
}

Option::Option(std::string textIn, std::list<std::string> resourcesIn, std::list<std::string> resourcesOut, std::string next)
{
	for (std::string s : resourcesIn)
	{
		resourcesNeeded.push_back(s);
	}
	for (std::string s : resourcesOut)
	{
		resourcesGiven.push_back(s);
	}
	nextRoom = next;
	active = false;
	text = textIn;
}
Option::~Option()
{

}

std::string Option::evaluate(std::vector<std::string> * resources)
{
	for (std::string s : resourcesNeeded)
	{
		//Remove the first occurence of the resource
		auto stringPos = std::find(resources->begin(), resources->end(), s);
		if (stringPos != resources->end()) resources->erase(stringPos);
	}

	for (std::string s : resourcesGiven)
	{
		resources->push_back(s);
	}

	return nextRoom;
}

//This DOES NOT work for when you need two of a resource!
bool Option::setActive(std::vector<std::string>* resources)
{
	for (std::string s : resourcesNeeded)
	{
		//Remove the first occurence of the resource
		auto stringPos = std::find(resources->begin(), resources->end(), s);
		if (stringPos == resources->end())
		{
			active = false;
			return false;
		}
	}
	active = true;
	return true;
}

std::string Option::getText(int index)
{
	return std::to_string(index + 1) + ": " + text;
}