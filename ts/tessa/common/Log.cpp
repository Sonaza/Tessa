#include "Precompiled.h"

#include <cstdio>
#include <cwchar>
#include <iostream>

#include <locale>
#include <codecvt>
#include <fstream>
#include <chrono>
#include <ctime>

#define PRINT_BUFFER_SIZE 4196

#if TS_PLATFORM == TS_WINDOWS
	#include "ts/tessa/common/IncludeWindows.h"
	#define localtime_r(_a, _b) localtime_s(_b, _a)
	#define sprintf sprintf_s
	#define vsprintf vswprintf_s
	#define vsnprintf vsnprintf_s
#endif

TS_PACKAGE1(log)

namespace
{

std::wofstream logStream;

}

void initialize()
{
	static const std::locale utf8_locale = std::locale(std::locale(), new std::codecvt_utf8<wchar_t>());

	logStream.imbue(utf8_locale);
	logStream.open("debug_output.log", std::ios::out | std::ios::trunc);

	TS_WPRINTF("Logging started\n");
}

void finalize()
{
	logStream.close();
}

void printf(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	printf(format, args);
	va_end(args);
}

void printf(const char* format, va_list args)
{
	char buffer[PRINT_BUFFER_SIZE] = { 0 };
	int32_t length = vsnprintf(buffer, PRINT_BUFFER_SIZE, format, args);

#if TS_PLATFORM == TS_WINDOWS
	OutputDebugStringA(buffer);
#endif

	if (logStream.is_open())
	{
		logStream << getTimestampStringWide() << " " << buffer;
		logStream.flush();
	}
}

void printf(const wchar_t* format, ...)
{
	va_list args;
	va_start(args, format);
	printf(format, args);
	va_end(args);
}

void printf(const wchar_t* format, va_list args)
{
	wchar_t buffer[PRINT_BUFFER_SIZE] = { 0 };
	int32_t length = vswprintf(buffer, PRINT_BUFFER_SIZE, format, args);

// 	std::u16string source = reinterpret_cast<char16_t*>(buffer);
// 	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
// 	std::string dest = convert.to_bytes(source);

#if TS_PLATFORM == TS_WINDOWS
	OutputDebugStringW(buffer);
#endif

// 	HANDLE stdhandle = GetStdHandle(STD_OUTPUT_HANDLE);
// 	DWORD n;
// 	WriteConsoleW(stdhandle, buffer, 1, &n, NULL);

	if (logStream.is_open())
	{
		logStream << getTimestampStringWide() << " " << buffer;
		logStream.flush();
	}
}

const std::string getTimestampString()
{
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

	std::chrono::system_clock::duration epoch = now.time_since_epoch();
	epoch -= std::chrono::duration_cast<std::chrono::seconds>(epoch);

	std::time_t rawtime = std::chrono::system_clock::to_time_t(now);

	std::tm time;
	localtime_r(&rawtime, &time);

	char buffer[26];
	sprintf(buffer, "[%04u-%02u-%02u %02u:%02u:%02u.%03u]",
		time.tm_year + 1900,
		time.tm_mon + 1,
		time.tm_mday,
		time.tm_hour,
		time.tm_min,
		time.tm_sec,
		std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count()
		);

	return std::string(buffer);
}

const std::wstring getTimestampStringWide()
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return std::wstring(converter.from_bytes(getTimestampString().c_str()));
}

TS_END_PACKAGE1()
