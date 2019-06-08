#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <type_traits>

TS_PACKAGE1(system)

class Commando
{
public:
	Commando();
	Commando(Int32 argc, const char **argv);

	void parse(Int32 argc, const char **argv);

	bool hasFlag(const std::string &flag) const;
	bool hasFlagParameter(const std::string &flag) const;

	template<class Type>
	bool getFlagParameter(const std::string &flag, Type &outParam) const;
	bool getFlagParameter(const std::string &flag, std::string &outParam) const;

	SizeType getNumParameters() const;

	template<class Type>
	bool getNthParameter(SizeType index, Type &outParam) const;
	bool getNthParameter(SizeType index, std::string &outParam);

	const std::string &getExecutablePath() const;

protected:
	std::string executablePath;

	typedef std::map<std::string, std::string> FlagsList;
	FlagsList flags;

	typedef std::vector<std::string> ParameterList;
	ParameterList parameters;
};

template<class Type>
bool Commando::getFlagParameter(const std::string &flag, Type &outParam) const
{
	static_assert(std::is_integral<Type>::value || std::is_floating_point<Type>::value, "Commando::getParameter can realistically only handle strings, integers and floating point values.");

	FlagsList::const_iterator iter = flags.find(flag);
	if (iter == flags.end() || iter->second.empty())
		return false;
	
	std::stringstream ss;
	ss << iter->second;
	ss >> outParam;
	return true;
}

template<class Type>
bool Commando::getNthParameter(SizeType index, Type &outParam) const
{
	static_assert(std::is_integral<Type>::value || std::is_floating_point<Type>::value, "Commando::getParameter can realistically only handle strings, integers and floating point values.");

	if (index >= parameters.size())
		return false;

	std::stringstream ss;
	ss << parameters[index];
	ss >> outParam;
	return true;
}

TS_END_PACKAGE1()
