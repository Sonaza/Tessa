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

	explicit Commando(const wchar_t *args);

	bool parse(int32 argc, const char **argv);
	bool parse(int32 argc, const wchar_t **argv);

	bool parse(const wchar_t *args);

	bool parse(const std::vector<String> &args);

	bool hasFlag(const String &flag) const;

	bool hasFlagParameter(const String &flag) const;

	template<class Type>
	bool getFlagParameter(const String &flag, Type &outParam) const;
	bool getFlagParameter(const String &flag, String &outParam) const;

	SizeType getNumParameters() const;

	template<class Type>
	bool getNthParameter(SizeType index, Type &outParam) const;
	bool getNthParameter(SizeType index, String &outParam) const;

protected:
	typedef std::unordered_map<uint32, String> FlagsList;
	FlagsList m_flags;

	typedef std::vector<String> ParameterList;
	ParameterList m_parameters;
};

template<class Type>
bool Commando::getFlagParameter(const String &flag, Type &outParam) const
{
	static_assert(std::is_integral<Type>::value || std::is_floating_point<Type>::value, "Commando::getParameter can realistically only handle strings, integers and floating point values.");

	FlagsList::const_iterator iter = m_flags.find(flag);
	if (iter == m_flags.end() || iter->second.empty())
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

	if (index >= m_parameters.size())
		return false;

	Stringstream ss;
	ss << m_parameters[index];
	ss >> outParam;
	return true;
}

TS_END_PACKAGE1()
