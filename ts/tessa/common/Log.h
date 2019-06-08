#pragma once

// Don't care about warnings in third-party code
#pragma warning( push, 3 )
#include <fmt/printf.h>
#pragma warning( pop )

#include <cstdarg>
#include <memory>
#include <fstream>

#define __TS_FMT_IMPL(__format, ...) ::ts::log::Log::getSingleton().format(__format, ## __VA_ARGS__)
#define TS_FMT(__format, ...) __TS_FMT_IMPL(__format, ## __VA_ARGS__)

#define __TS_PRINTF_IMPL(__format, ...) \
	do { \
		::ts::log::Log::getSingleton().write(__format, ## __VA_ARGS__); \
	} while(false)

#define __TS_WPRINTF_IMPL(__format, ...) \
	do { \
		::ts::log::Log::getSingleton().write(L ## __format, ## __VA_ARGS__); \
	} while(false)

#if TS_BUILD != TS_FINALRELEASE
	#define TS_PRINTF(__format, ...)  __TS_PRINTF_IMPL(__format, ## __VA_ARGS__)
	#define TS_WPRINTF(__format, ...) __TS_WPRINTF_IMPL(__format, ## __VA_ARGS__)
#else
	#define TS_PRINTF(...)  ((void)0)
	#define TS_WPRINTF(...) ((void)0)
#endif

#define __TS_LOG_WITH_SEVERITY_IMPL(__severity, __message, ...) \
	do { \
		__TS_PRINTF_IMPL("[%s] %s - %s:%u in function %s\n", __severity, TS_FMT(__message, ##__VA_ARGS__), __FILE__, __LINE__, TS_PRETTY_FUNCTION); \
	} while(false)

#define TS_LOG_ERROR(__message, ...)   __TS_LOG_WITH_SEVERITY_IMPL("Error", __message, ##__VA_ARGS__)
#define TS_LOG_WARNING(__message, ...) __TS_LOG_WITH_SEVERITY_IMPL("Warning", __message, ##__VA_ARGS__)
#define TS_LOG_INFO(__message, ...)    __TS_LOG_WITH_SEVERITY_IMPL("Info", __message, ##__VA_ARGS__)

#if TS_BUILD != TS_FINALRELEASE
	#define TS_LOG_DEBUG(__message, ...) __TS_LOG_WITH_SEVERITY_IMPL("Debug", __message, ##__VA_ARGS__)
#else
	#define TS_LOG_DEBUG(__message, ...) ((void)0)
#endif

TS_PACKAGE1(log)

class Log
{
public:
	static Log &getSingleton();

	void write(const std::string &str);
	void write(const std::wstring &str);

	template<class FormatType, class... Args>
	void write(FormatType format, Args... args);
	
	template<class... Args>
	const std::string format(const std::string &format, Args... args);

	template<class... Args>
	const std::wstring format(const std::wstring &format, Args... args);

	const std::string makeTimestampString() const;
	const std::wstring makeTimestampStringWide() const;

private:
	Log();
	~Log();

	Log(Log const&) = delete;
	void operator=(Log const&) = delete;
	
	std::wofstream logFileStream;
};

template<class FormatType, class... Args>
void Log::write(FormatType format, Args... args)
{
	write(fmt::sprintf(format, args...));
}

template<class... Args>
const std::string Log::format(const std::string &format, Args... args)
{
	return fmt::sprintf(format, args...);
}

template<class... Args>
const std::wstring Log::format(const std::wstring &format, Args... args)
{
	return fmt::sprintf(format, args...);
}

TS_END_PACKAGE1()

