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

/* Modified for Tessa engine use.
 */

#include "Precompiled.h"
#include "String.h"

#include "ts/string/Utf.h"

#include <iterator>
#include <cstring>

TS_PACKAGE1(string)

Character::Character(char ansiChar, const std::locale &locale)
	: m_utf32Char(Utf32::decodeAnsi(ansiChar, locale))
{
}

Character::Character(wchar_t wideChar)
	: m_utf32Char(Utf32::decodeWide(wideChar))
{
}

Character::Character(char32_t utf32Char)
	: m_utf32Char(utf32Char)
{
}

char Character::toAnsiChar(const std::locale &locale) const
{
	char output = 0;
	Utf32::encodeAnsi(m_utf32Char, &output, 0, locale);
	return output;
}

wchar_t Character::toWideChar() const
{
	wchar_t output = 0;
	Utf32::encodeWide(m_utf32Char, &output, 0);
	return output;
}

Character::operator char32_t() const
{
	return m_utf32Char;
}

/*************************/

const BigSizeType String::InvalidPos = std::basic_string<char32_t>::npos;

String::String()
{
}

String::String(char ansiChar, const std::locale &locale)
{
	m_buffer += Utf32::decodeAnsi(ansiChar, locale);
}

String::String(wchar_t wideChar)
{
	m_buffer += Utf32::decodeWide(wideChar);
}

String::String(char32_t utf32Char)
{
	m_buffer += utf32Char;
}

String::String(const char *ansiString, const std::locale &locale)
{
	if (ansiString)
	{
		BigSizeType length = strlen(ansiString);
		if (length > 0)
		{
			m_buffer.reserve(length + 1);
			Utf32::fromAnsi(ansiString, ansiString + length, std::back_inserter(m_buffer), locale);
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
	m_buffer.reserve(ansiString.length() + 1);
	Utf32::fromAnsi(ansiString.begin(), ansiString.end(), std::back_inserter(m_buffer), locale);
}

String::String(const wchar_t *wideString)
{
	if (wideString)
	{
		BigSizeType length = std::wcslen(wideString);
		if (length > 0)
		{
			m_buffer.reserve(length + 1);
			Utf32::fromWide(wideString, wideString + length, std::back_inserter(m_buffer));
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
	m_buffer.reserve(wideString.length() + 1);
	Utf32::fromWide(wideString.begin(), wideString.end(), std::back_inserter(m_buffer));
}

String::String(const char32_t *utf32String)
{
	if (utf32String)
		m_buffer = utf32String;
}

String::String(const char32_t *utf32String, BigSizeType size)
	: m_buffer(utf32String, size)
{
}

String::String(const std::basic_string<char32_t> &utf32String)
	: m_buffer(utf32String)
{
}

String::String(const String &other)
	: m_buffer(other.m_buffer)
{
}

String &String::operator=(const String &other)
{
	m_buffer = other.m_buffer;
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
		m_buffer = std::move(other.m_buffer);
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

#if TS_GLOBAL_USING_SFML == TS_TRUE

String::operator sf::String() const
{
	return sf::String(reinterpret_cast<const uint32 *>(getPointer()));
}

#endif

std::string String::toAnsiString(const std::locale &locale) const
{
	// Prepare the output string
	std::string output;
	output.reserve(m_buffer.length() + 1);
	
	// Convert
	Utf32::toAnsi(m_buffer.begin(), m_buffer.end(), std::back_inserter(output), 0, locale);
	
	return output;
}

std::wstring String::toWideString() const
{
	// Prepare the output string
	std::wstring output;
	output.reserve(m_buffer.length() + 1);
	
	// Convert
	Utf32::toWide(m_buffer.begin(), m_buffer.end(), std::back_inserter(output), 0);
	
	return output;
}

std::basic_string<char> String::toUtf8() const
{
	// Prepare the output string
	std::basic_string<char> output;
	output.reserve(m_buffer.length());
	
	// Convert
	Utf32::toUtf8(m_buffer.begin(), m_buffer.end(), std::back_inserter(output));
	
	return output;
}

std::basic_string<char16_t> String::toUtf16() const
{
	// Prepare the output string
	std::basic_string<char16_t> output;
	output.reserve(m_buffer.length());
	
	// Convert
	Utf32::toUtf16(m_buffer.begin(), m_buffer.end(), std::back_inserter(output));
	
	return output;
}

std::basic_string<char32_t> String::toUtf32() const
{
	return m_buffer;
}

char32_t &String::operator[](BigSizeType index)
{
	TS_ASSERT(index < (BigSizeType)m_buffer.size());
	return m_buffer[index];
}

const char32_t &String::operator[](BigSizeType index) const
{
	TS_ASSERT(index < (BigSizeType)m_buffer.size());
	return m_buffer[index];
}

Character String::front() const
{
	return m_buffer.front();
}

Character String::back() const
{
	return m_buffer.back();
}

void String::reserve(BigSizeType size)
{
	m_buffer.reserve(size);
}

void String::clear()
{
	m_buffer.clear();
}

bool String::isEmpty() const
{
	return m_buffer.empty();
}

BigSizeType String::getSize() const
{
	return m_buffer.size();
}

void String::erase(BigSizeType position, BigSizeType count)
{
	m_buffer.erase(position, count);
}

void String::erase(const_iterator pos)
{
	m_buffer.erase(pos);
}

void String::erase(const_iterator first, const_iterator last)
{
	m_buffer.erase(first, last);
}

void String::append(const String &str)
{
	m_buffer += str.m_buffer;
}

void String::append(const Character &chr)
{
	m_buffer += chr;
}

void String::insert(BigSizeType position, const String &str)
{
	m_buffer.insert(position, str.m_buffer);
}

void String::insert(const_iterator pos, const String &str)
{
	m_buffer.insert(pos, str.begin(), str.end());
}

BigSizeType String::find(const String &str, BigSizeType start) const
{
	return m_buffer.find(str.m_buffer, start);
}

BigSizeType String::findFirstOf(const Character &chr, BigSizeType start) const
{
	return m_buffer.find_first_of((char32_t)chr, start);
}

BigSizeType String::findFirstOf(const String &str, BigSizeType start) const
{
	return m_buffer.find_first_of(str.m_buffer, start);
}

BigSizeType String::findFirstNotOf(const Character &chr, BigSizeType start) const
{
	return m_buffer.find_first_not_of((char32_t)chr, start);
}

BigSizeType String::findFirstNotOf(const String &str, BigSizeType start) const
{
	return m_buffer.find_first_not_of(str.m_buffer, start);
}

BigSizeType String::findLastOf(const Character &chr, BigSizeType start) const
{
	return m_buffer.find_last_of((char32_t)chr, start);
}

BigSizeType String::findLastOf(const String &str, BigSizeType start) const
{
	return m_buffer.find_last_of(str.m_buffer, start);
}

BigSizeType String::findLastNotOf(const Character &chr, BigSizeType start) const
{
	return m_buffer.find_last_not_of((char32_t)chr, start);
}

BigSizeType String::findLastNotOf(const String &str, BigSizeType start) const
{
	return m_buffer.find_last_not_of(str.m_buffer, start);
}

void String::replace(BigSizeType position, BigSizeType length, const String &replaceWith)
{
	m_buffer.replace(position, length, replaceWith.m_buffer);
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
	return m_buffer.substr(position, length);
}

const char32_t *String::getPointer() const
{
	return m_buffer.c_str();
}

String::iterator String::begin()
{
	return m_buffer.begin();
}

String::iterator String::end()
{
	return m_buffer.end();
}

String::const_iterator String::begin() const
{
	return m_buffer.begin();
}

String::const_iterator String::end() const
{
	return m_buffer.end();
}

String::reverse_iterator String::rbegin()
{
	return m_buffer.rbegin();
}

String::reverse_iterator String::rend()
{
	return m_buffer.rend();
}

String::const_reverse_iterator String::rbegin() const
{
	return m_buffer.rbegin();
}

String::const_reverse_iterator String::rend() const
{
	return m_buffer.rend();
}

bool operator==(const String &left, const String &right)
{
	return left.m_buffer == right.m_buffer;
}

bool operator!=(const String &left, const String &right)
{
	return !(left == right);
}

bool operator<(const String &left, const String &right)
{
	return left.m_buffer < right.m_buffer;
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
	string.append(right);
	return string;
}

String operator+(const Character &left, const String &right)
{
	String string(left);
	string.append(right);
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
