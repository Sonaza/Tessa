#include "Precompiled.h"

#include <iostream>
#include <locale>
#include <codecvt>
#include <chrono>
#include <ctime>

#include <streambuf>
#include <array>

#include "ts/lang/AppConfig.h"
#include "ts/lang/common/Assert.h"
#include "ts/string/StringUtils.h"
#include "ts/file/FileUtils.h"
#include "ts/thread/MutexGuard.h"
#include "ts/thread/RecursiveMutex.h"

#if TS_PLATFORM == TS_WINDOWS
#include "ts/lang/common/IncludeWindows.h"
#define localtime_r(_a, _b) localtime_s(_b, _a)
#endif

#if TS_GLOBAL_USING_SFML == TS_TRUE
#include "SFML/System/Err.hpp"
#endif

TS_PACKAGE1(common)

class CustomLoggerBuffer : public std::streambuf
{
public:
	CustomLoggerBuffer(const std::string &messageTag, bool assertOnOutput)
		: messageTag(messageTag)
		, assertOnOutput(assertOnOutput)
	{
		memset(&_buffer[0], 0, _buffer.size());
		char *base = &_buffer[0];
		setp(base, base + _buffer.size() - 1);
	}

protected:
	virtual int32 overflow(int32 ch) override
	{
		if (ch != traits_type::eof())
		{
			*pptr() = (char)ch;
			pbump(1);
		}
		return traits_type::eof();
	}

	int32 sync() override
	{
		if (pbase() != pptr())
		{
			String str(pbase(), pptr());
			setp(pbase(), epptr());

			string::trimWhitespace(str);

			std::string message = TS_FMT("[%s] %s\n", messageTag, str);
			TS_PRINTF(message);

			TS_ASSERTF(!assertOnOutput, message.c_str());
		}
		return 0;
	}

private:
	static const BigSizeType BUFFER_MAX_SIZE = 1024;
	std::array<char, BUFFER_MAX_SIZE> _buffer;
	std::string messageTag;
	bool assertOnOutput = false;
};

class CustomLoggerOutputStream : public std::ostream, private virtual CustomLoggerBuffer
{
public:
	CustomLoggerOutputStream(const std::string &messageTag, bool assertOnOutput)
		: CustomLoggerBuffer(messageTag, assertOnOutput)
		, std::ostream(static_cast<std::streambuf*>(this))
	{
		flags(std::ios_base::unitbuf);
	}
};

static RecursiveMutex mutex;

static String defaultFilepath = file::joinPaths(file::getExecutableDirectory(), getApplicationLogFile());

static String currentFilepath;
static std::ofstream outputFileStream;

class LogStaticInitializer
{
public:
	LogStaticInitializer()
	{
		Log::openLogfile();

#if TS_GLOBAL_USING_SFML == TS_TRUE
		static CustomLoggerOutputStream customLogger("SFML Error", true);
		sf::err().rdbuf(customLogger.rdbuf());
#endif
	}

	~LogStaticInitializer()
	{
		Log::closeLogfile();
	}
};
static LogStaticInitializer logStaticInitializer;

bool Log::isLogFileOpen()
{
	MutexGuard mg(mutex);
	return outputFileStream.is_open() && outputFileStream.good();
}

bool Log::openLogfile()
{
	static const std::locale utf8_locale = std::locale(std::locale(), new std::codecvt_utf8<wchar_t>());
	outputFileStream.imbue(utf8_locale);

#if TS_PLATFORM == TS_WINDOWS
	outputFileStream.open(defaultFilepath.toWideString(), std::ios::out | std::ios::trunc);
#else
	outputFileStream.open(defaultFilepath.toUtf8().c_str(), std::ios::out | std::ios::trunc);
#endif

	if (!outputFileStream)
	{
		TS_LOG_ERROR("Opening log file for writing failed! File path: %s", defaultFilepath);
		return false;
	}

	currentFilepath = defaultFilepath;

	return true;
}

void Log::closeLogfile()
{
	MutexGuard mg(mutex);

	if (outputFileStream.is_open())
		outputFileStream.close();
}

void Log::write(const String &str)
{
	MutexGuard mg(mutex);

#if TS_PLATFORM == TS_WINDOWS
	std::wcout << str;
	OutputDebugStringW(str.toWideString().c_str());
#else
	std::cout << str.toUtf8().c_str();
#endif

	if (outputFileStream)
	{
		outputFileStream << makeTimestampString() << " " << str.toUtf8().c_str();
		outputFileStream.flush();
	}
}

String Log::makeTimestampString()
{
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

	std::chrono::system_clock::duration epoch = now.time_since_epoch();
	epoch -= std::chrono::duration_cast<std::chrono::seconds>(epoch);

	std::time_t rawtime = std::chrono::system_clock::to_time_t(now);

	std::tm time;
	localtime_r(&rawtime, &time);

	return String(fmt::sprintf("[%04u-%02u-%02u %02u:%02u:%02u.%03u]",
		time.tm_year + 1900,
		time.tm_mon + 1,
		time.tm_mday,
		time.tm_hour,
		time.tm_min,
		time.tm_sec,
		std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count()
	));
}


TS_END_PACKAGE1()
