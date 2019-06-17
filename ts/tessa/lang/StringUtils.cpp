#include "Precompiled.h"
#include "ts/tessa/lang/StringUtils.h"

#include <algorithm>
#include <cctype>

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

extern std::vector<std::string> splitString(const std::string &str, const std::string &delimiters)
{
	std::vector<std::string> tokens;
	size_t index = 0;
	while (index < str.size())
	{
		size_t pos = str.find_first_of(&delimiters[0], index, delimiters.size());
		tokens.push_back(str.substr(index, pos - index));
		if (pos == std::string::npos)
			break;

		index = pos + 1;
	}
	return tokens;
}

TS_END_PACKAGE2()
