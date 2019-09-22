////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2018 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

/* Modified for Tessa engine use. Using internal types and namespacing.
 */

#pragma once

 // Don't care about warnings in third-party code
#pragma warning( push, 3 )
#include <fmt/format.h>
#include <fmt/printf.h>
#pragma warning( pop )

#include "ts/tessa/common/Assert.h"
#include "ts/tessa/string/Utf.h"

#include <iterator>
#include <locale>
#include <string>

TS_PACKAGE1(string)

class Character
{
public:
	Character(char ansiChar, const std::locale &locale = std::locale());
	Character(wchar_t wideChar);
	Character(char32_t utf32Char);

	char toAnsiChar(const std::locale &locale = std::locale()) const;
	wchar_t toWideChar() const;

	operator char32_t() const;

private:
	char32_t utf32Char = 0;
};

class String
{
public:
	typedef std::basic_string<char32_t>::iterator               iterator;
	typedef std::basic_string<char32_t>::const_iterator         const_iterator;

	typedef std::basic_string<char32_t>::reverse_iterator       reverse_iterator; 
	typedef std::basic_string<char32_t>::const_reverse_iterator const_reverse_iterator;

	static const BigSizeType InvalidPos;

	String();

	explicit String(char ansiChar, const std::locale &locale = std::locale());
	explicit String(wchar_t wideChar);
	explicit String(char32_t utf32Char);

	String(const char *ansiString, const std::locale &locale = std::locale());
	String(const char *ansiString, BigSizeType size, const std::locale &locale = std::locale());
	String(const char *first, const char *last, const std::locale &locale = std::locale());
	String(const std::string &ansiString, const std::locale &locale = std::locale());
	
	String(const wchar_t *wideString);
	String(const wchar_t *wideString, BigSizeType size);
	String(const wchar_t *first, const wchar_t *last);
	String(const std::wstring &wideString);
	
	String(const char32_t *utf32String);
	String(const char32_t *utf32String, BigSizeType size);
	String(const std::basic_string<char32_t> &utf32String);

	String(const String &other);
	String &operator=(const String &other);
	
	String(String &&other);
	String &operator=(String &&other);

	template <typename T>
	static String fromUtf8(T begin, T end);

	template <typename T>
	static String fromUtf16(T begin, T end);

	template <typename T>
	static String fromUtf32(T begin, T end);

	operator std::string() const;
	operator std::wstring() const;

#if TS_GLOBAL_USING_SFML == TS_TRUE
	operator sf::String() const;
#endif

	/* String formatting as provided by fmt-library.
	 * sprintf is the old school POSIX style formatting.
	 * format is the new school Python style formatting.
	*/ 
	template<class... Args>
	String &sprintf(const std::string &formatStr, const Args&... args);

	template<class... Args>
	String &sprintf(const std::wstring &formatStr, const Args&... args);

	template<class... Args>
	String &format(const std::string &formatStr, const Args&... args);

	template<class... Args>
	String &format(const std::wstring &formatStr, const Args&... args);

	std::string toAnsiString(const std::locale &locale = std::locale()) const;
	std::wstring toWideString() const;

	std::basic_string<char> toUtf8() const;
	std::basic_string<char16_t> toUtf16() const;
	std::basic_string<char32_t> toUtf32() const;

	char32_t &operator[](BigSizeType index);
	const char32_t &operator[](BigSizeType index) const;

	Character front() const;
	Character back() const;

	void reserve(BigSizeType size);
	void clear();

	bool isEmpty() const;
	BigSizeType getSize() const;

	void erase(BigSizeType position, BigSizeType count = 1);
	void erase(const_iterator pos);
	void erase(const_iterator first, const_iterator last);

	void append(const String &str);
	void append(const Character &chr);
	void insert(BigSizeType position, const String &str);
	void insert(const_iterator pos, const String &str);

	BigSizeType find(const String &str, BigSizeType start = 0) const;

	BigSizeType findFirstOf(const Character &chr, BigSizeType start = 0) const;
	BigSizeType findFirstOf(const String &str, BigSizeType start = 0) const;

	BigSizeType findFirstNotOf(const Character &chr, BigSizeType start = 0) const;
	BigSizeType findFirstNotOf(const String &str, BigSizeType start = 0) const;

	/* Start offset is counted from beginning. Using InvalidPos starts the search from the end.
	*/ 
	BigSizeType findLastOf(const Character &chr, BigSizeType start = String::InvalidPos) const;
	BigSizeType findLastOf(const String &str, BigSizeType start = String::InvalidPos) const;

	BigSizeType findLastNotOf(const Character &chr, BigSizeType start = String::InvalidPos) const;
	BigSizeType findLastNotOf(const String &str, BigSizeType start = String::InvalidPos) const;
	
	void replace(BigSizeType position, BigSizeType length, const String &replaceWith);
	void replace(const String &searchFor, const String &replaceWith);

	String substring(BigSizeType position, BigSizeType length = InvalidPos) const;

	const char32_t *getPointer() const;

	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;

	reverse_iterator rbegin();
	reverse_iterator rend();
	const_reverse_iterator rbegin() const;
	const_reverse_iterator rend() const;

private:
	friend bool operator==(const String &left, const String &right);
	friend bool operator<(const String &left, const String &right);

	// Internal string of UTF-32 characters
	std::basic_string<char32_t> buffer;
};

bool operator==(const String &left, const String &right);
bool operator!=(const String &left, const String &right);
bool operator<(const String &left, const String &right);
bool operator>(const String &left, const String &right);
bool operator<=(const String &left, const String &right);
bool operator>=(const String &left, const String &right);
String operator+(const String &left, const String &right);
String operator+(const Character &left, const String &right);
String operator+(const String &left, const Character &right);

std::basic_ostream<char> &operator<<(std::basic_ostream<char> &strm, const String &str);
std::basic_ostream<wchar_t> &operator<<(std::basic_ostream<wchar_t> &strm, const String &str);

template <typename T>
String String::fromUtf8(T begin, T end)
{
	String string;
	Utf8::toUtf32(begin, end, std::back_inserter(string.buffer));
	return string;
}

template <typename T>
String String::fromUtf16(T begin, T end)
{
	String string;
	Utf16::toUtf32(begin, end, std::back_inserter(string.buffer));
	return string;
}

template <typename T>
String String::fromUtf32(T begin, T end)
{
	String string;
	string.buffer.assign(begin, end);
	return string;
}

template<class... Args>
String &String::sprintf(const std::string &formatStr, const Args&... args)
{
	try
	{
		*this = String(fmt::sprintf(formatStr, args...));
	}
	catch (const fmt::format_error &e)
	{
		TS_UNUSED_VARIABLE(e);
		TS_ASSERTF(false, "String formatting error: %s", e.what());
		*this = String("<formatting error>");
	}
	return *this;
}

template<class... Args>
String &String::sprintf(const std::wstring &formatStr, const Args&... args)
{
	try
	{
		*this = String(fmt::sprintf(formatStr, args...));
	}
	catch (const fmt::format_error &e)
	{
		TS_UNUSED_VARIABLE(e);
		TS_ASSERTF(false, "String formatting error: %s", e.what());
		*this = String("<formatting error>");
	}
	return *this;
}

template<class... Args>
String &String::format(const std::string &formatStr, const Args&... args)
{
	try
	{
		*this = String(fmt::format(formatStr, args...));
	}
	catch (const fmt::format_error &e)
	{
		TS_UNUSED_VARIABLE(e);
		TS_ASSERTF(false, "String formatting error: %s", e.what());
		*this = String("<formatting error>");
	}
	return *this;
}

template<class... Args>
String &String::format(const std::wstring &formatStr, const Args&... args)
{
	try
	{
		*this = String(fmt::format(formatStr, args...));
	}
	catch (const fmt::format_error &e)
	{
		TS_UNUSED_VARIABLE(e);
		TS_ASSERTF(false, "String formatting error: %s", e.what());
		*this = String("<formatting error>");
	}
	return *this;
}


TS_END_PACKAGE1()

TS_PACKAGE0()

using string::String;

TS_END_PACKAGE0()

// namespace std
// {
// 
// template<>
// struct hash<ts::string::String>
// {
// 	size_t operator()(ts::string::String const& s) const noexcept
// 	{
// 		return std::hash<std::basic_string<char32_t>>{}(s.getData());
// 	}
// };
// 
// }
