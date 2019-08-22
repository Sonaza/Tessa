#pragma once

#include <string>
#include <vector>

TS_PACKAGE2(lang, utils)

template<class CharType, class TraitsType, class AllocatorType>
struct GetStringCharTypeImpl
{
	typedef CharType CharType;
};

template<class StringType>
struct GetStringCharType
{
	typedef decltype(StringType()[0]) type;
};

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

// extern std::vector<std::string> splitString(const std::string &str, const std::string &delimiters);

extern std::vector<std::string> splitString(const std::string &str, const std::string &delimiters);
extern std::vector<std::wstring> splitString(const std::wstring &str, const std::wstring &delimiters);

extern std::string joinString(const std::vector<std::string> &arr, const std::string &glue);
extern std::wstring joinString(const std::vector<std::wstring> &arr, const std::wstring &glue);

// template<class CharType>
// std::basic_string<CharType, std::char_traits<CharType>, std::allocator<CharType>>
// 	joinString(
// 		const std::vector<std::basic_string<CharType, std::char_traits<CharType>, std::allocator<CharType>>> &arr,
// 		const std::basic_string<CharType, std::char_traits<CharType>, std::allocator<CharType>> &glue)
// {
// 	typedef std::basic_string<CharType, std::char_traits<CharType>, std::allocator<CharType>> StringType;
// 	typedef std::basic_stringstream<CharType, std::char_traits<CharType>, std::allocator<CharType>> StringStreamType;
// 
// 	const StringType emptyStr;
// 	StringStreamType ss;
// 	for (auto it = arr.begin(); it != arr.end(); ++it)
// 	{
// 		ss << (it != arr.begin() ? glue : emptyStr) << *it;
// 	}
// 	return ss.str();
// }

TS_END_PACKAGE2()
