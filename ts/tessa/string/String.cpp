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

#include "Precompiled.h"

#include "ts/tessa/string/String.h"
#include "ts/tessa/string/Utf.h"

#include <iterator>
#include <cstring>

TS_PACKAGE1(string)

Character::Character(char ansiChar, const std::locale &locale)
	: utf32Char(Utf32::decodeAnsi(ansiChar, locale))
{
}

Character::Character(wchar_t wideChar)
	: utf32Char(Utf32::decodeWide(wideChar))
{
}

Character::Character(char32_t utf32Char)
	: utf32Char(utf32Char)
{
}

char Character::toAnsiChar(const std::locale &locale) const
{
	char output = 0;
	Utf32::encodeAnsi(utf32Char, &output, 0, locale);
	return output;
}

wchar_t Character::toWideChar() const
{
	wchar_t output = 0;
	Utf32::encodeWide(utf32Char, &output, 0);
	return output;
}

Character::operator char32_t() const
{
	return utf32Char;
}

/*************************/

const BigSizeType String::InvalidPos = std::basic_string<char32_t>::npos;

String::String()
{
}

String::String(char ansiChar, const std::locale &locale)
{
	buffer += Utf32::decodeAnsi(ansiChar, locale);
}

String::String(wchar_t wideChar)
{
	buffer += Utf32::decodeWide(wideChar);
}

String::String(char32_t utf32Char)
{
	buffer += utf32Char;
}

String::String(const char *ansiString, const std::locale &locale)
{
	if (ansiString)
	{
		BigSizeType length = strlen(ansiString);
		if (length > 0)
		{
			buffer.reserve(length + 1);
			Utf32::fromAnsi(ansiString, ansiString + length, std::back_inserter(buffer), locale);
		}
	}
}

String::String(const char *first, BigSizeType size, const std::locale &locale)
	: String(std::string(first, size), locale)
{
}

String::String(const char *first, const char *last, const std::locale &locale)
	: String(std::string(first, last), locale)
{
}

String::String(const std::string &ansiString, const std::locale &locale)
{
	buffer.reserve(ansiString.length() + 1);
	Utf32::fromAnsi(ansiString.begin(), ansiString.end(), std::back_inserter(buffer), locale);
}

String::String(const wchar_t *wideString)
{
	if (wideString)
	{
		BigSizeType length = std::wcslen(wideString);
		if (length > 0)
		{
			buffer.reserve(length + 1);
			Utf32::fromWide(wideString, wideString + length, std::back_inserter(buffer));
		}
	}
}

String::String(const wchar_t *first, BigSizeType size)
	: String(std::wstring(first, size))
{
}

String::String(const wchar_t *first, const wchar_t *last)
	: String(std::wstring(first, last))
{
}

String::String(const std::wstring &wideString)
{
	buffer.reserve(wideString.length() + 1);
	Utf32::fromWide(wideString.begin(), wideString.end(), std::back_inserter(buffer));
}

String::String(const char32_t *utf32String)
{
	if (utf32String)
		buffer = utf32String;
}

String::String(const char32_t *utf32String, BigSizeType size)
	: buffer(utf32String, size)
{
}

String::String(const std::basic_string<char32_t> &utf32String)
	: buffer(utf32String)
{
}

String::String(const String &other)
	: buffer(other.buffer)
{
}

String &String::operator=(const String &other)
{
	buffer = other.buffer;
	return *this;
}

String::String(String &&other)
{
	*this = std::move(other);
}

String &String::operator=(String &&other)
{
	if (this != &other)
	{
		buffer = std::move(other.buffer);
	}
	return *this;
}

String::operator std::string() const
{
	return toAnsiString();
}

String::operator std::wstring() const
{
	return toWideString();
}

#if defined(SFML_VERSION_MAJOR)

String::operator sf::String() const
{
	return sf::String(reinterpret_cast<const uint32 *>(getData()));
}

#endif

std::string String::toAnsiString(const std::locale &locale) const
{
	// Prepare the output string
	std::string output;
	output.reserve(buffer.length() + 1);
	
	// Convert
	Utf32::toAnsi(buffer.begin(), buffer.end(), std::back_inserter(output), 0, locale);
	
	return output;
}

std::wstring String::toWideString() const
{
	// Prepare the output string
	std::wstring output;
	output.reserve(buffer.length() + 1);
	
	// Convert
	Utf32::toWide(buffer.begin(), buffer.end(), std::back_inserter(output), 0);
	
	return output;
}

std::basic_string<char> String::toUtf8() const
{
	// Prepare the output string
	std::basic_string<char> output;
	output.reserve(buffer.length());
	
	// Convert
	Utf32::toUtf8(buffer.begin(), buffer.end(), std::back_inserter(output));
	
	return output;
}

std::basic_string<char16_t> String::toUtf16() const
{
	// Prepare the output string
	std::basic_string<char16_t> output;
	output.reserve(buffer.length());
	
	// Convert
	Utf32::toUtf16(buffer.begin(), buffer.end(), std::back_inserter(output));
	
	return output;
}

std::basic_string<char32_t> String::toUtf32() const
{
	return buffer;
}

String &String::operator+=(const String &right)
{
	buffer += right.buffer;
	return *this;
}

char32_t &String::operator[](BigSizeType index)
{
	TS_ASSERT(index < buffer.size());
	return buffer[index];
}

const char32_t &String::operator[](BigSizeType index) const
{
	TS_ASSERT(index < buffer.size());
	return buffer[index];
}

Character String::front() const
{
	return buffer.front();
}

Character String::back() const
{
	return buffer.back();
}

void String::reserve(BigSizeType size)
{
	buffer.reserve(size);
}

void String::clear()
{
	buffer.clear();
}

BigSizeType String::getSize() const
{
	return buffer.size();
}

bool String::isEmpty() const
{
	return buffer.empty();
}

void String::erase(BigSizeType position, BigSizeType count)
{
	buffer.erase(position, count);
}

void String::erase(const_iterator pos)
{
	buffer.erase(pos);
}

void String::erase(const_iterator first, const_iterator last)
{
	buffer.erase(first, last);
}

void String::append(const String &str)
{
	buffer += str.buffer;
}

void String::append(const Character &chr)
{
	buffer += chr;
}

void String::insert(BigSizeType position, const String &str)
{
	buffer.insert(position, str.buffer);
}

void String::insert(const_iterator pos, const String &str)
{
	buffer.insert(pos, str.begin(), str.end());
}

BigSizeType String::find(const String &str, BigSizeType start) const
{
	return buffer.find(str.buffer, start);
}

BigSizeType String::findFirstOf(const Character &chr, BigSizeType start) const
{
	return buffer.find_first_of((char32_t)chr, start);
}

BigSizeType String::findFirstOf(const String &str, BigSizeType start) const
{
	return buffer.find_first_of(str.buffer, start);
}

BigSizeType String::findFirstNotOf(const Character &chr, BigSizeType start) const
{
	return buffer.find_first_not_of((char32_t)chr, start);
}

BigSizeType String::findFirstNotOf(const String &str, BigSizeType start) const
{
	return buffer.find_first_not_of(str.buffer, start);
}

BigSizeType String::findLastOf(const Character &chr, BigSizeType start) const
{
	return buffer.find_last_of((char32_t)chr, start);
}

BigSizeType String::findLastOf(const String &str, BigSizeType start) const
{
	return buffer.find_last_of(str.buffer, start);
}

BigSizeType String::findLastNotOf(const Character &chr, BigSizeType start) const
{
	return buffer.find_last_not_of((char32_t)chr, start);
}

BigSizeType String::findLastNotOf(const String &str, BigSizeType start) const
{
	return buffer.find_last_not_of(str.buffer, start);
}

void String::replace(BigSizeType position, BigSizeType length, const String &replaceWith)
{
	buffer.replace(position, length, replaceWith.buffer);
}

void String::replace(const String &searchFor, const String &replaceWith)
{
	BigSizeType step = replaceWith.getSize();
	BigSizeType len = searchFor.getSize();
	BigSizeType pos = find(searchFor);
	
	// Replace each occurrence of search
	while (pos != InvalidPos)
	{
		replace(pos, len, replaceWith);
		pos = find(searchFor, pos + step);
	}
}

String String::substring(BigSizeType position, BigSizeType length) const
{
	return buffer.substr(position, length);
}

const char32_t *String::getData() const
{
	return buffer.c_str();
}

String::iterator String::begin()
{
	return buffer.begin();
}

String::iterator String::end()
{
	return buffer.end();
}

String::const_iterator String::begin() const
{
	return buffer.begin();
}

String::const_iterator String::end() const
{
	return buffer.end();
}

String::reverse_iterator String::rbegin()
{
	return buffer.rbegin();
}

String::reverse_iterator String::rend()
{
	return buffer.rend();
}

String::const_reverse_iterator String::rbegin() const
{
	return buffer.rbegin();
}

String::const_reverse_iterator String::rend() const
{
	return buffer.rend();
}

bool operator==(const String &left, const String &right)
{
	return left.buffer == right.buffer;
}

bool operator!=(const String &left, const String &right)
{
	return !(left == right);
}

bool operator<(const String &left, const String &right)
{
	return left.buffer < right.buffer;
}

bool operator>(const String &left, const String &right)
{
	return right < left;
}

bool operator<=(const String &left, const String &right)
{
	return !(right < left);
}

bool operator>=(const String &left, const String &right)
{
	return !(left < right);
}

String operator+(const String &left, const String &right)
{
	String string = left;
	string += right;
	return string;
}

String operator+(const Character &left, const String &right)
{
	String string(left);
	string += right;
	return string;
}

String operator+(const String &left, const Character &right)
{
	String string = left;
	string.append(right);
	return string;
}

std::basic_ostream<char> &operator<<(std::basic_ostream<char> &strm, const String &str)
{
	strm << str.toAnsiString();
	return strm;
}

std::basic_ostream<wchar_t> &operator<<(std::basic_ostream<wchar_t> &strm, const String &str)
{
	strm << str.toWideString();
	return strm;
}

TS_END_PACKAGE1()
