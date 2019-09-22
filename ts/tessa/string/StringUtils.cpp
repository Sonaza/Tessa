#include "Precompiled.h"
#include "StringUtils.h"

#include <algorithm>
#include <cctype>
#include <sstream>

TS_PACKAGE1(string)

// Removes whitespace on the left in place
static void leftTrim(String &str)
{
	str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](uint32 c)
	{
		return !std::isspace(c);
	}));
}

// Removes whitespace on the right in place
static void rightTrim(String &str)
{
	str.erase(std::find_if(str.rbegin(), str.rend(), [](int32 c)
	{
		return !std::isspace(c);
	}).base(), str.end());
}

extern void toUppercase(String &str)
{
	std::transform(str.begin(), str.end(), str.begin(), std::toupper);
}

extern void toLowercase(String &str)
{
	std::transform(str.begin(), str.end(), str.begin(), std::tolower);
}

extern String toUppercaseCopy(const String &str)
{
	String out = str;
	toUppercase(out);
	return out;
}

extern String toLowercaseCopy(const String &str)
{
	String out = str;
	toLowercase(out);
	return out;
}

extern void trimWhitespace(String &str)
{
	leftTrim(str);
	rightTrim(str);
}

extern String trimWhitespaceCopy(const String &str)
{
	String out = str;
	trimWhitespace(out);
	return out;
}

extern void replaceCharacter(String &str, Character search, Character replacement)
{
	std::replace(str.begin(), str.end(), search, replacement);
}

extern String replaceCharacterCopy(const String &str, Character search, Character replacement)
{
	String out = str;
	replaceCharacter(out, search, replacement);
	return out;
}

std::vector<String> splitString(const String &str, Character delimiter)
{
	return splitString(str, String(delimiter));
}

std::vector<String> splitString(const String &str, const String &delimiters)
{
	std::vector<String> tokens;
	size_t index = 0;
	while (index < str.getSize())
	{
		size_t pos = str.findFirstOf(delimiters, index);
		tokens.push_back(str.substring(index, pos - index));
		if (pos == String::InvalidPos)
			break;

		index = pos + 1;
	}
	return tokens;
}

extern String joinString(const std::vector<String> &arr, const String &glue)
{
	BigSizeType totalSize = 0;
	for (const String &str : arr)
		totalSize += str.getSize() + glue.getSize();

	String result;
	result.reserve(totalSize);

	for (auto it = arr.begin(); it != arr.end(); ++it)
	{
		result.append((it != arr.begin() ? glue : "") + *it);
	}
	return result;
}

TS_END_PACKAGE1()
