#include "Precompiled.h"

#include <iostream>
#include <locale>
#include <codecvt>
#include <chrono>
#include <ctime>

#include "ts/tessa/Config.h"

#if TS_PLATFORM == TS_WINDOWS
	#include "ts/tessa/common/IncludeWindows.h"
	#define localtime_r(_a, _b) localtime_s(_b, _a)
#endif

TS_PACKAGE1(log)

std::string Log::filepath = DEFAULT_LOG_FILE_NAME;

Log &Log::getSingleton()
{
	static Log instance;
	return instance;
}

bool Log::setLogFile(const std::string &filepathParam)
{
	if (filepath.empty())
		return false;

	Log::filepath = filepathParam;

	Log &log = getSingleton();
	if (log.openLogfile())
		return false;
	
	return true;
}

bool Log::isLogFileOpen() const
{
	return fileStream.is_open() && fileStream.good();
}

Log::Log()
{
	openLogfile();
}

Log::~Log()
{
	closeLogfile();
}

bool Log::openLogfile()
{
	TS_ASSERT(!Log::filepath.empty());

	// Check that the requested file isn't already opened
	if (Log::filepath == currentFilepath && isLogFileOpen())
		return true;

	closeLogfile();

	static const std::locale utf8_locale = std::locale(std::locale(), new std::codecvt_utf8<wchar_t>());
	fileStream.imbue(utf8_locale);
	fileStream.open(Log::filepath, std::ios::out | std::ios::trunc);

	if (!isLogFileOpen())
	{
		TS_LOG_ERROR("Opening log file for writing failed! File path: %s", filepath);
		return false;
	}

	currentFilepath = Log::filepath;

	fileStream << "THIS IS FIRST LINE\n";
	fileStream.flush();

	return true;
}

void Log::closeLogfile()
{
	if (fileStream.is_open())
		fileStream.close();
}

void Log::write(const std::string &str)
{
	std::cout << str;

#if TS_PLATFORM == TS_WINDOWS
	OutputDebugStringA(str.c_str());
#endif

	if (isLogFileOpen())
	{
		fileStream << makeTimestampStringWide() << " " << str.c_str();
		fileStream.flush();
	}
}

void Log::write(const std::wstring &str)
{
	std::wcout << str;

#if TS_PLATFORM == TS_WINDOWS
	OutputDebugStringW(str.c_str());
#endif

	if (isLogFileOpen())
	{
		fileStream << makeTimestampStringWide() << " " << str.c_str();
		fileStream.flush();
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
