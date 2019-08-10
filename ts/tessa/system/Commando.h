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
	Commando(Int32 argc, const char **argv);
	Commando(Int32 argc, const wchar_t **argv);

	void parse(Int32 argc, const char **argv);
	void parse(Int32 argc, const wchar_t **argv);

	bool hasFlag(const std::string &flag) const;
	bool hasFlag(const std::wstring &flag) const;

	bool hasFlagParameter(const std::string &flag) const;
	bool hasFlagParameter(const std::wstring &flag) const;

	template<class Type>
	bool getFlagParameter(const std::string &flag, Type &outParam) const;
	bool getFlagParameter(const std::string &flag, std::string &outParam) const;
	bool getFlagParameter(const std::wstring &flag, std::wstring &outParam) const;

	SizeType getNumParameters() const;

	template<class Type>
	bool getNthParameter(SizeType index, Type &outParam) const;
	bool getNthParameter(SizeType index, std::string &outParam) const;
	bool getNthParameter(SizeType index, std::wstring &outParam) const;

	const std::string &getExecutablePath() const;
	const std::wstring &getExecutablePathWide() const;

protected:
	std::string executablePath_utf8;
	std::wstring executablePath_utf16;

	typedef std::unordered_map<Uint32, std::string> FlagsList;
	FlagsList flags;

	typedef std::vector<std::string> ParameterList;
	ParameterList parameters;

	std::wstring convertU8toU16(const std::string &str) const;
	std::string convertU16toU8(const std::wstring &str) const;

	std::vector<std::string> rawArgumentsList;
	std::vector<std::wstring> rawArgumentsListW;
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
