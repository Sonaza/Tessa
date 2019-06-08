#include "Precompiled.h"

#include <iostream>
#include <locale>
#include <codecvt>
#include <chrono>
#include <ctime>

#if TS_PLATFORM == TS_WINDOWS
	#include "ts/tessa/common/IncludeWindows.h"
	#define localtime_r(_a, _b) localtime_s(_b, _a)
#endif

TS_PACKAGE1(log)

Log &Log::getSingleton()
{
	static Log instance;
	return instance;
}

Log::Log()
{
	static const std::locale utf8_locale = std::locale(std::locale(), new std::codecvt_utf8<wchar_t>());

	logFileStream.imbue(utf8_locale);
	logFileStream.open("RuntimeOutput.log", std::ios::out | std::ios::trunc);
}

Log::~Log()
{
	logFileStream.close();
}

void Log::write(const std::string &str)
{
	std::cout << str;

#if TS_PLATFORM == TS_WINDOWS
	OutputDebugStringA(str.c_str());
#endif

	if (logFileStream.is_open())
	{
		logFileStream << makeTimestampStringWide() << " " << str.c_str();
		logFileStream.flush();
	}
}

void Log::write(const std::wstring &str)
{
	std::wcout << str;

#if TS_PLATFORM == TS_WINDOWS
	OutputDebugStringW(str.c_str());
#endif

	if (logFileStream.is_open())
	{
		logFileStream << makeTimestampStringWide() << " " << str.c_str();
		logFileStream.flush();
	}
}

const std::string Log::makeTimestampString() const
{
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

	std::chrono::system_clock::duration epoch = now.time_since_epoch();
	epoch -= std::chrono::duration_cast<std::chrono::seconds>(epoch);

	std::time_t rawtime = std::chrono::system_clock::to_time_t(now);

	std::tm time;
	localtime_r(&rawtime, &time);

	return fmt::sprintf("[%04u-%02u-%02u %02u:%02u:%02u.%03u]",
		time.tm_year + 1900,
		time.tm_mon + 1,
		time.tm_mday,
		time.tm_hour,
		time.tm_min,
		time.tm_sec,
		std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count()
	);
}

const std::wstring Log::makeTimestampStringWide() const
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return std::wstring(converter.from_bytes(makeTimestampString().c_str()));
}

TS_END_PACKAGE1()
