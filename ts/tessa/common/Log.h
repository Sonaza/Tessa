#pragma once

// Don't care about warnings in third-party code
#pragma warning( push, 3 )
#include <fmt/printf.h>
#pragma warning( pop )

#include "ts/tessa/common/Assert.h"

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

	static bool setLogFile(const std::string &filepath);

	bool isLogFileOpen() const;

	void write(const std::string &str);
	void write(const std::wstring &str);

	template<class FormatType, class... Args>
	void write(FormatType format, Args... args);
	
	template<class... Args>
	std::string format(const std::string &format, Args... args);

	template<class... Args>
	std::wstring format(const std::wstring &format, Args... args);

	const std::string makeTimestampString() const;
	const std::wstring makeTimestampStringWide() const;

private:
	Log();
	~Log();

	Log(Log const&) = delete;
	void operator=(Log const&) = delete;

	bool openLogfile();
	void closeLogfile();
	
	std::string currentFilepath;
	std::wofstream fileStream;

	static std::string filepathToBeOpened;
};

template<class FormatType, class... Args>
TS_FORCEINLINE void Log::write(FormatType formatStr, Args... args)
{
	write(format(formatStr, args...));
}

template<class... Args>
std::string Log::format(const std::string &formatStr, Args... args)
{
	try
	{
		return fmt::sprintf(formatStr, args...);
	}
	catch (const fmt::v5::format_error &e)
	{
		TS_UNUSED_VARIABLE(e);
		TS_ASSERTF(false, "String formatting error: %s", e.what());
		return "<formatting error>";
	}
}

template<class... Args>
std::wstring Log::format(const std::wstring &formatStr, Args... args)
{
	try
	{
		return fmt::sprintf(formatStr, args...);
	}
	catch (const fmt::v5::format_error &e)
	{
		TS_UNUSED_VARIABLE(e);
		TS_ASSERTF(false, "String formatting error: %s", e.what());
		return L"<formatting error>";
	}
}

TS_END_PACKAGE1()

