#include "Precompiled.h"
#include "ts/tessa/lang/StringUtils.h"

#include <algorithm>
#include <cctype>
#include <sstream>

TS_PACKAGE2(lang, utils)

namespace
{

// Removes whitespace on the left in place
void leftTrim(std::string &str)
{
	str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](Int32 c)
	{
		return !std::isspace(c);
	}));
}

// Removes whitespace on the right in place
void rightTrim(std::string &str)
{
	str.erase(std::find_if(str.rbegin(), str.rend(), [](Int32 c)
	{
		return !std::isspace(c);
	}).base(), str.end());
}

}

extern void toUppercase(std::string &str)
{
	std::transform(str.begin(), str.end(), str.begin(), std::toupper);
}

extern void toLowercase(std::string &str)
{
	std::transform(str.begin(), str.end(), str.begin(), std::tolower);
}

extern std::string toUppercaseCopy(const std::string &str)
{
	std::string out = str;
	toUppercase(out);
	return out;
}

extern std::string toLowercaseCopy(const std::string &str)
{
	std::string out = str;
	toLowercase(out);
	return out;
}

extern void trimWhitespace(std::string &str)
{
	leftTrim(str);
	rightTrim(str);
}

extern std::string trimWhitespaceCopy(const std::string &str)
{
	std::string out = str;
	trimWhitespace(out);
	return out;
}

extern void removeCharacter(std::string &str, const char removedCharacter)
{
	str.erase(std::remove(str.begin(), str.end(), removedCharacter), str.end());
}

extern std::string removeCharacterCopy(const std::string &str, const char removedCharacter)
{
	std::string out = str;
	removeCharacter(out, removedCharacter);
	return out;
}

extern void replaceCharacter(std::string &str, const char search, const char replacement)
{
	std::replace(str.begin(), str.end(), search, replacement);
}

extern std::string replaceCharacterCopy(const std::string &str, const char search, const char replacement)
{
	std::string out = str;
	replaceCharacter(out, search, replacement);
	return out;
}

template<class StringType>
std::vector<StringType> splitString(const StringType &str, const StringType &delimiters)
{
	std::vector<StringType> tokens;
	size_t index = 0;
	while (index < str.size())
	{
		size_t pos = str.find_first_of(&delimiters[0], index, delimiters.size());
		tokens.push_back(str.substr(index, pos - index));
		if (pos == StringType::npos)
			break;

		index = pos + 1;
	}
	return tokens;
}

std::vector<std::string> splitString(const std::string &str, const std::string &delimiters)
{
	typedef std::string StringType;

	std::vector<StringType> tokens;
	size_t index = 0;
	while (index < str.size())
	{
		size_t pos = str.find_first_of(&delimiters[0], index, delimiters.size());
		tokens.push_back(str.substr(index, pos - index));
		if (pos == StringType::npos)
			break;

		index = pos + 1;
	}
	return tokens;
}

std::vector<std::wstring> splitString(const std::wstring &str, const std::wstring &delimiters)
{
	typedef std::wstring StringType;

	std::vector<StringType> tokens;
	size_t index = 0;
	while (index < str.size())
	{
		size_t pos = str.find_first_of(&delimiters[0], index, delimiters.size());
		tokens.push_back(str.substr(index, pos - index));
		if (pos == StringType::npos)
			break;

		index = pos + 1;
	}
	return tokens;
}

extern std::string joinString(const std::vector<std::string> &arr, const std::string &glue)
{
	std::stringstream ss;
	for (auto it = arr.begin(); it != arr.end(); ++it)
	{
		ss << (it != arr.begin() ? glue : "") << *it;
	}
	return ss.str();
}

extern std::wstring joinString(const std::vector<std::wstring> &arr, const std::wstring &glue)
{
	std::wstringstream ss;
	for (auto it = arr.begin(); it != arr.end(); ++it)
	{
		ss << (it != arr.begin() ? glue : L"") << *it;
	}
	return ss.str();
}

TS_END_PACKAGE2()
