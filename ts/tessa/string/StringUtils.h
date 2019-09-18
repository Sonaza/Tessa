#pragma once

#include "ts/tessa/string/String.h"
#include <vector>

TS_PACKAGE1(string)

// template<class StringType>
// struct GetStringCharType
// {
// 	typedef decltype(StringType()[0]) type;
// };

extern void toUppercase(String &str);
extern void toLowercase(String &str);
extern String toUppercaseCopy(const String &str);
extern String toLowercaseCopy(const String &str);

extern void trimWhitespace(String &str);
extern String trimWhitespaceCopy(const String &str);

extern void replaceCharacter(String &str, Character search, Character replacement);
extern String replaceCharacterCopy(const String &str, Character search, Character replacement);

extern std::vector<String> splitString(const String &str, Character delimiter);
extern std::vector<String> splitString(const String &str, const String &delimiters);

extern String joinString(const std::vector<String> &arr, const String &glue);

TS_END_PACKAGE1()
