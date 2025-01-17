#include "Precompiled.h"
#include "StringUtils.h"

#include <algorithm>
#include <cctype>
#include <sstream>

TS_PACKAGE1(string)

// Removes whitespace on the left in place
static void leftTrim(String &str)
{
	str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](uint32_t c)
	{
		return !std::isspace(c);
	}));
}

// Removes whitespace on the right in place
static void rightTrim(String &str)
{
	str.erase(std::find_if(str.rbegin(), str.rend(), [](int32_t c)
	{
		return !std::isspace(c);
	}).base(), str.end());
}

extern void toUppercase(String &str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::toupper);
}

extern void toLowercase(String &str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
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
	BigSizeType index = 0;
	while (index < str.getSize())
	{
		BigSizeType pos = str.findFirstOf(delimiters, index);
		tokens.push_back(str.substring(index, pos - index));
		if (pos == String::InvalidPos)
			break;

		index = pos + 1;
	}
	return tokens;
}

extern bool startsWith(const String &str, const String &needle)
{
	if (str.getSize() < needle.getSize())
		return false;

	return str.substring(0, needle.getSize()) == needle;
}

extern bool endsWith(const String &str, const String &needle)
{
	if (str.getSize() < needle.getSize())
		return false;

	return str.substring(str.getSize() - needle.getSize()) == needle;
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

extern String truncateString(const String &str, SizeType maxSize, const String &appendStr)
{
	if (str.getSize() > maxSize)
		return str.substring(0, maxSize) + appendStr;
	return str;
}

TS_END_PACKAGE1()
