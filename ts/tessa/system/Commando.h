#pragma once

#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <type_traits>

TS_PACKAGE1(system)

class Commando
{
public:
	Commando();
	Commando(int32 argc, const char **argv);
	Commando(int32 argc, const wchar_t **argv);

	void parse(int32 argc, const char **argv);
	void parse(int32 argc, const wchar_t **argv);

	bool hasFlag(const String &flag) const;

	bool hasFlagParameter(const String &flag) const;

	template<class Type>
	bool getFlagParameter(const String &flag, Type &outParam) const;
	bool getFlagParameter(const String &flag, String &outParam) const;

	SizeType getNumParameters() const;

	template<class Type>
	bool getNthParameter(SizeType index, Type &outParam) const;
	bool getNthParameter(SizeType index, String &outParam) const;

	const String &getExecutablePath() const;

protected:
	String executablePath;

	typedef std::unordered_map<uint32, String> FlagsList;
	FlagsList flags;

	typedef std::vector<String> ParameterList;
	ParameterList parameters;

	std::vector<String> rawArgumentsList;
};

template<class Type>
bool Commando::getFlagParameter(const String &flag, Type &outParam) const
{
	static_assert(std::is_integral<Type>::value || std::is_floating_point<Type>::value, "Commando::getParameter can realistically only handle strings, integers and floating point values.");

	FlagsList::const_iterator iter = flags.find(flag);
	if (iter == flags.end() || iter->second.empty())
		return false;
	
	Stringstream ss;
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

	Stringstream ss;
	ss << parameters[index];
	ss >> outParam;
	return true;
}

TS_END_PACKAGE1()
