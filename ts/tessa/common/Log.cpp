#include "Precompiled.h"

#include <iostream>
#include <locale>
#include <codecvt>
#include <chrono>
#include <ctime>

#include <streambuf>

#include "ts/tessa/common/Assert.h"
#include "ts/tessa/string/StringUtils.h"
#include "ts/tessa/Config.h"

#include <array>

#if TS_PLATFORM == TS_WINDOWS
#include "ts/tessa/common/IncludeWindows.h"
#define localtime_r(_a, _b) localtime_s(_b, _a)
#endif

TS_PACKAGE1(common)

class CustomLoggerBuffer : public std::streambuf
{
public:
	CustomLoggerBuffer(const std::string &messageTag, bool assertOnError)
		: _messageTag(messageTag)
		, assertOnError(assertOnError)
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
// 		return this->sync() ? traits_type::not_eof(ch) : traits_type::eof();
		return traits_type::eof();
	}

	int32 sync()
	{
		if (pbase() != pptr())
		{
			String str(pbase(), pptr());
			setp(pbase(), epptr());

			string::trimWhitespace(str);

			std::string message = TS_FMT("[%s] %s\n", _messageTag, str);
			TS_PRINTF(message);
			TS_ASSERTF(!assertOnError, message.c_str());
		}
		return 0;
	}

private:
	enum { BUFFER_MAX_SIZE = 1024, };
	std::array<char, BUFFER_MAX_SIZE> _buffer;
	std::string _messageTag;
	bool assertOnError = false;
};

class CustomLoggerOutputStream : public std::ostream, private virtual CustomLoggerBuffer
{
public:
	CustomLoggerOutputStream(const std::string &messageTag, bool assertOnError)
		: CustomLoggerBuffer(messageTag, assertOnError)
		, std::ostream(static_cast<std::streambuf*>(this))
	{
		flags(std::ios_base::unitbuf);
	}
};

String Log::filepathToBeOpened = TS_DEFAULT_LOG_FILE_NAME;

Log &Log::getSingleton()
{
	static Log instance;
	return instance;
}

bool Log::setLogFile(const String &filepathParam)
{
	if (filepathToBeOpened.isEmpty())
		return false;

	Log::filepathToBeOpened = filepathParam;

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

	static CustomLoggerOutputStream customLogger("SFML Error", true);
	sf::err().rdbuf(customLogger.rdbuf());

}

Log::~Log()
{
	closeLogfile();
}

bool Log::openLogfile()
{
	TS_ASSERT(!Log::filepathToBeOpened.isEmpty());

	// Check that the requested file isn't already opened
	if (Log::filepathToBeOpened == currentFilepath && isLogFileOpen())
		return true;

	closeLogfile();

	static const std::locale utf8_locale = std::locale(std::locale(), new std::codecvt_utf8<wchar_t>());
	fileStream.imbue(utf8_locale);

#if TS_PLATFORM == TS_WINDOWS
	fileStream.open(Log::filepathToBeOpened.toWideString(), std::ios::out | std::ios::trunc);
#else
	fileStream.open(Log::filepathToBeOpened.toUtf8().c_str(), std::ios::out | std::ios::trunc);
#endif

	if (!isLogFileOpen())
	{
		TS_LOG_ERROR("Opening log file for writing failed! File path: %s", filepathToBeOpened);
		return false;
	}

	currentFilepath = Log::filepathToBeOpened;

	return true;
}

void Log::closeLogfile()
{
	if (fileStream.is_open())
		fileStream.close();
}

void Log::write(const String &str)
{
#if TS_PLATFORM == TS_WINDOWS
	std::wcout << str;
	OutputDebugStringW(str.toWideString().c_str());
#else
	std::cout << str.toUtf8().c_str();
#endif

	if (isLogFileOpen())
	{
		fileStream << makeTimestampString() << " " << str.toUtf8().c_str();
		fileStream.flush();
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
