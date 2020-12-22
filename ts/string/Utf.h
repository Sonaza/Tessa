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

#include <algorithm>
#include <locale>
#include <string>
#include <cstdlib>

TS_PACKAGE1(string)

template <unsigned int N>
class Utf;

template <>
class Utf<8>
{
public:

	template <typename In>
	static In decode(In begin, In end, uint32_t &output, uint32_t replacement = 0);

	template <typename Out>
	static Out encode(uint32_t input, Out output, uint8_t replacement = 0);

	template <typename In>
	static In next(In begin, In end);

	template <typename In>
	static std::size_t count(In begin, In end);

	template <typename In, typename Out>
	static Out fromAnsi(In begin, In end, Out output, const std::locale &locale = std::locale());

	template <typename In, typename Out>
	static Out fromWide(In begin, In end, Out output);

	template <typename In, typename Out>
	static Out fromLatin1(In begin, In end, Out output);

	template <typename In, typename Out>
	static Out toAnsi(In begin, In end, Out output, char replacement = 0, const std::locale &locale = std::locale());

	template <typename In, typename Out>
	static Out toWide(In begin, In end, Out output, wchar_t replacement = 0);

	template <typename In, typename Out>
	static Out toLatin1(In begin, In end, Out output, char replacement = 0);

	template <typename In, typename Out>
	static Out toUtf8(In begin, In end, Out output);

	template <typename In, typename Out>
	static Out toUtf16(In begin, In end, Out output);

	template <typename In, typename Out>
	static Out toUtf32(In begin, In end, Out output);
};

template <>
class Utf<16>
{
public:

	template <typename In>
	static In decode(In begin, In end, uint32_t &output, uint32_t replacement = 0);

	template <typename Out>
	static Out encode(uint32_t input, Out output, uint16_t replacement = 0);

	template <typename In>
	static In next(In begin, In end);

	template <typename In>
	static std::size_t count(In begin, In end);

	template <typename In, typename Out>
	static Out fromAnsi(In begin, In end, Out output, const std::locale &locale = std::locale());

	template <typename In, typename Out>
	static Out fromWide(In begin, In end, Out output);

	template <typename In, typename Out>
	static Out fromLatin1(In begin, In end, Out output);

	template <typename In, typename Out>
	static Out toAnsi(In begin, In end, Out output, char replacement = 0, const std::locale &locale = std::locale());

	template <typename In, typename Out>
	static Out toWide(In begin, In end, Out output, wchar_t replacement = 0);

	template <typename In, typename Out>
	static Out toLatin1(In begin, In end, Out output, char replacement = 0);

	template <typename In, typename Out>
	static Out toUtf8(In begin, In end, Out output);

	template <typename In, typename Out>
	static Out toUtf16(In begin, In end, Out output);

	template <typename In, typename Out>
	static Out toUtf32(In begin, In end, Out output);
};

template <>
class Utf<32>
{
public:

	template <typename In>
	static In decode(In begin, In end, uint32_t &output, uint32_t replacement = 0);

	template <typename Out>
	static Out encode(uint32_t input, Out output, uint32_t replacement = 0);

	template <typename In>
	static In next(In begin, In end);

	template <typename In>
	static std::size_t count(In begin, In end);

	template <typename In, typename Out>
	static Out fromAnsi(In begin, In end, Out output, const std::locale &locale = std::locale());

	template <typename In, typename Out>
	static Out fromWide(In begin, In end, Out output);

	template <typename In, typename Out>
	static Out fromLatin1(In begin, In end, Out output);

	template <typename In, typename Out>
	static Out toAnsi(In begin, In end, Out output, char replacement = 0, const std::locale &locale = std::locale());

	template <typename In, typename Out>
	static Out toWide(In begin, In end, Out output, wchar_t replacement = 0);

	template <typename In, typename Out>
	static Out toLatin1(In begin, In end, Out output, char replacement = 0);

	template <typename In, typename Out>
	static Out toUtf8(In begin, In end, Out output);

	template <typename In, typename Out>
	static Out toUtf16(In begin, In end, Out output);

	template <typename In, typename Out>
	static Out toUtf32(In begin, In end, Out output);

	template <typename In>
	static uint32_t decodeAnsi(In input, const std::locale &locale = std::locale());

	template <typename In>
	static uint32_t decodeWide(In input);

	template <typename Out>
	static Out encodeAnsi(uint32_t codepoint, Out output, char replacement = 0, const std::locale &locale = std::locale());

	template <typename Out>
	static Out encodeWide(uint32_t codepoint, Out output, wchar_t replacement = 0);
};

#include "Utf.inl"

// Make typedefs to get rid of the template syntax
typedef Utf<8>  Utf8;
typedef Utf<16> Utf16;
typedef Utf<32> Utf32;

TS_END_PACKAGE1()
