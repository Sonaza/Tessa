#pragma once

#include "ts/tessa/string/String.h"

#include <ostream>
#include <cstdarg>
#include <memory>
#include <fstream>

#define TS_FMT(__format, ...) \
	::ts::common::Log::getSingleton().format(__format, ## __VA_ARGS__)

#define TS_WFMT(__format, ...) \
	::ts::common::Log::getSingleton().format(L ## __format, ## __VA_ARGS__)

#define __TS_PRINTF_IMPL(__format, ...) \
	::ts::common::Log::getSingleton().write(__format, ## __VA_ARGS__)

#define __TS_WPRINTF_IMPL(__format, ...) \
	::ts::common::Log::getSingleton().write(L ## __format, ## __VA_ARGS__)

#if TS_BUILD != TS_FINALRELEASE
	#define TS_PRINTF(__format, ...)  __TS_PRINTF_IMPL(__format, ## __VA_ARGS__)
	#define TS_WPRINTF(__format, ...) __TS_WPRINTF_IMPL(__format, ## __VA_ARGS__)
#else
	#define TS_PRINTF(...)  ((void)0)
	#define TS_WPRINTF(...) ((void)0)
#endif

#define __TS_WIDE(text) L##text
#define TS_WIDE(text) __TS_WIDE(text)

#define __TS_LOG_WITH_SEVERITY_IMPL(__severity, __message, ...) \
	do { \
		__TS_PRINTF_IMPL("[%s] %s - %s\n", __severity, TS_FUNCTION_LOG_SIMPLE, TS_FMT(__message, ##__VA_ARGS__)/*, __FILE__, __LINE__*/); \
	} while(false)

#define __TS_WLOG_WITH_SEVERITY_IMPL(__severity, __message, ...) \
	do { \
		__TS_WPRINTF_IMPL("[%s] %s - %s\n", __severity, TS_WIDE(TS_FUNCTION_LOG_SIMPLE), TS_FMT(TS_WIDE(__message), ##__VA_ARGS__)/*, __FILE__, __LINE__*/); \
	} while(false)

#define TS_LOG_ERROR(__message, ...)   __TS_LOG_WITH_SEVERITY_IMPL("Error", __message, ##__VA_ARGS__)
#define TS_LOG_WARNING(__message, ...) __TS_LOG_WITH_SEVERITY_IMPL("Warning", __message, ##__VA_ARGS__)
#define TS_LOG_INFO(__message, ...)    __TS_LOG_WITH_SEVERITY_IMPL("Info", __message, ##__VA_ARGS__)

#define TS_WLOG_ERROR(__message, ...)   __TS_WLOG_WITH_SEVERITY_IMPL(L"Error", __message, ##__VA_ARGS__)
#define TS_WLOG_WARNING(__message, ...) __TS_WLOG_WITH_SEVERITY_IMPL(L"Warning", __message, ##__VA_ARGS__)
#define TS_WLOG_INFO(__message, ...)    __TS_WLOG_WITH_SEVERITY_IMPL(L"Info", __message, ##__VA_ARGS__)

#if TS_BUILD != TS_FINALRELEASE
	#define TS_LOG_DEBUG(__message, ...)  __TS_LOG_WITH_SEVERITY_IMPL("Debug", __message, ##__VA_ARGS__)
	#define TS_WLOG_DEBUG(__message, ...) __TS_WLOG_WITH_SEVERITY_IMPL(L"Debug", __message, ##__VA_ARGS__)
#else
	#define TS_LOG_DEBUG(__message, ...) ((void)0)
	#define TS_WLOG_DEBUG(__message, ...) ((void)0)
#endif

#include "ts/tessa/common/Package.h"

TS_PACKAGE1(common)

class Log
{
public:
	static Log &getSingleton();

	static bool setLogFile(const String &filepath);

	bool isLogFileOpen() const;

	void write(const String &str);

	template<class... Args>
	void write(const std::string &format, const Args&... args);

	template<class... Args>
	void write(const std::wstring &format, const Args&... args);

	template<class... Args>
	String format(const std::string &format, const Args&... args);

	template<class... Args>
	String format(const std::wstring &format, const Args&... args);

	String makeTimestampString();

private:
	Log();
	~Log();

	Log(const Log&) = delete;
	void operator=(const Log&) = delete;

	bool openLogfile();
	void closeLogfile();
	
	String currentFilepath;
	std::ofstream fileStream;

	static String filepathToBeOpened;
};

// template<class FormatType, class... Args>
// TS_FORCEINLINE void Log::write(FormatType formatStr, const Args&... args)
// {
// 	write(format(formatStr, args...));
// }

template<class... Args>
TS_FORCEINLINE void Log::write(const std::string &formatStr, const Args&... args)
{
	write(format(formatStr, args...));
}

template<class... Args>
TS_FORCEINLINE void Log::write(const std::wstring &formatStr, const Args&... args)
{
	write(format(formatStr, args...));
}

template<class... Args>
String Log::format(const std::string &formatStr, const Args&... args)
{
	return String().sprintf(formatStr, args...);
}

template<class... Args>
String Log::format(const std::wstring &formatStr, const Args&... args)
{
	return String().sprintf(formatStr, args...);
}

TS_END_PACKAGE1()

