#pragma once

#include <sstream>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "ts/math/Hash.h"

TS_PACKAGE2(engine, system)

class Commando
{
public:
	Commando();
	~Commando();

	/* Parse overload to simplify passing a raw argument list to the actual parser.
	 *
	 * Does a separate parsing for the raw argument list by taking quotation marks
	 * into account. Currently backslash escaping isn't supported.
	 */
	bool parse(const wchar_t *args);

	/* Parse overload to simplify passing an argument list to the actual parser.
	 * 
	 * Note: if passing values from CRT entry point like main(), the first argv should be skipped
	 *       and argc subtracted by 1. The first value (execution path) isn't always useful
	 *       or reliable, and isn't present on all platforms when using different entry point.
	 */
	bool parse(int32 argc, const char **argv);
	bool parse(int32 argc, const wchar_t **argv);

	/* Parses argument list in a way where:
	 * 1) all segments without preceding dash(es) are considered raw parameters.
	 * 2) all segments with prepended by a single dash are considered flags with 1 parameter (the next segment).
	 * 3) all segments with prepended by multiple dashes are considered flags with no parameters.
	 * Returns true if parsing is successful.
	 */
	bool parse(const std::vector<String> &args);

	/* Returns if a given flag is set and exists.
	 */
	bool hasFlag(const String &flag) const;

	/* Returns if a given flag exists and has a parameter.
	 */
	bool hasFlagParameter(const String &flag) const;

	/* Retrieve a flag parameter by the flag.
	 * Parameter is set to the outParam variable.
	 * Returns false if flag does not exist or parameter is empty.
	 */
	template<class Type>
	bool getFlagParameter(const String &flag, Type &outParam) const;
	bool getFlagParameter(const String &flag, String &outParam) const;

	/* Returns number of non-flag parameters.
	 */
	SizeType getNumParameters() const;

	/* Retrieve the parameter value by index.
	 * Parameter is set to the outParam variable.
	 * Returns false if index is out of range.
	 */
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
	static_assert(std::is_integral<Type>::value || std::is_floating_point<Type>::value, "Commando::getParameter can only handle strings, integers and floating point values.");

	uint32 flagHash = math::simpleHash32(flag);
	FlagsList::const_iterator iter = m_flags.find(flagHash);
	if (iter == m_flags.end() || iter->second.isEmpty())
		return false;
	
	// Using string stream to perform lexical casting
	std::stringstream ss;
	ss << iter->second;
	ss >> outParam;
	return true;
}

template<class Type>
bool Commando::getNthParameter(SizeType index, Type &outParam) const
{
	static_assert(std::is_integral<Type>::value || std::is_floating_point<Type>::value, "Commando::getParameter can only handle strings, integers and floating point values.");

	if (index >= m_parameters.size())
		return false;

	// Using string stream to perform lexical casting
	std::stringstream ss;
	ss << m_parameters[index];
	ss >> outParam;
	return true;
}

TS_END_PACKAGE2()
