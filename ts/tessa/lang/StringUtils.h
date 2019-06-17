#pragma once

#include <string>

TS_PACKAGE2(lang, utils)

extern void toUppercase(std::string &str);
extern void toLowercase(std::string &str);
extern std::string toUppercaseCopy(const std::string &str);
extern std::string toLowercaseCopy(const std::string &str);

extern void trimWhitespace(std::string &str);
extern std::string trimWhitespaceCopy(const std::string &str);

extern void removeCharacter(std::string &str, const char removedCharacter);
extern std::string removeCharacterCopy(const std::string &str, const char removedCharacter);

extern void replaceCharacter(std::string &str, const char search, const char replacement);
extern std::string replaceCharacterCopy(const std::string &str, const char search, const char replacement);

extern std::vector<std::string> splitString(const std::string &str, const std::string &delimiters);

TS_END_PACKAGE2()
