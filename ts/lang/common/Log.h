#pragma once

#include "ts/string/String.h"

#include <ostream>
#include <cstdarg>
#include <memory>
#include <fstream>

#define TS_FMT(formatParam, ...) \
	::ts::common::Log::getSingleton().format(formatParam, ## __VA_ARGS__)

#define TS_PRINTF_IMPL(formatParam, ...) \
	::ts::common::Log::getSingleton().write(formatParam, ## __VA_ARGS__)
	
#if TS_BUILD != TS_FINALRELEASE
	#define TS_PRINTF(formatParam, ...)  TS_PRINTF_IMPL(formatParam, ## __VA_ARGS__)
#else
	#define TS_PRINTF(...)  ((void)0)
#endif
	

#if TS_PLATFORM == TS_WINDOWS
	
	#define TS_WFMT(formatParam, ...) \
		::ts::common::Log::getSingleton().format(L ## formatParam, ## __VA_ARGS__)

	#define TS_WPRINTF_IMPL(formatParam, ...) \
		::ts::common::Log::getSingleton().write(L ## formatParam, ## __VA_ARGS__)

	#if TS_BUILD != TS_FINALRELEASE
		#define TS_WPRINTF(formatParam, ...) TS_WPRINTF_IMPL(formatParam, ## __VA_ARGS__)
	#else
		#define TS_WPRINTF(...) ((void)0)
	#endif

	#define TS_WIDE_INNER(textParam) L##textParam
	#define TS_WIDE(textParam) TS_WIDE_INNER(textParam)
		
#else
	
	#define TS_WFMT(formatParam, ...) TS_FMT(formatParam, __VA_ARGS__)
	#define TS_WPRINTF(formatParam, ...) TS_PRINTF(formatParam, __VA_ARGS__)
	
	#define TS_WIDE(textParam) (textParam)
	
#endif

#define TS_LOG_WITH_SEVERITY_IMPL(severityParam, messageParam, ...) \
	do { \
		TS_PRINTF_IMPL("[%s] %s - %s\n", severityParam, TS_FUNCTION_LOG_SIMPLE, TS_FMT(messageParam, ##__VA_ARGS__)/*, __FILE__, __LINE__*/); \
	} while(false)

#define TS_LOG_ERROR(messageParam, ...)   TS_LOG_WITH_SEVERITY_IMPL("Error", messageParam, ##__VA_ARGS__)
#define TS_LOG_WARNING(messageParam, ...) TS_LOG_WITH_SEVERITY_IMPL("Warning", messageParam, ##__VA_ARGS__)
#define TS_LOG_INFO(messageParam, ...)    TS_LOG_WITH_SEVERITY_IMPL("Info", messageParam, ##__VA_ARGS__)

#if TS_BUILD != TS_FINALRELEASE
	#define TS_LOG_DEBUG(messageParam, ...)  TS_LOG_WITH_SEVERITY_IMPL("Debug", messageParam, ##__VA_ARGS__)
#else
	#define TS_LOG_DEBUG(messageParam, ...) ((void)0)
#endif

#if TS_PLATFORM == TS_WINDOWS
	
	#define TS_WLOG_WITH_SEVERITY_IMPL(severityParam, messageParam, ...) \
	do { \
		TS_WPRINTF_IMPL("[%s] %s - %s\n", severityParam, TS_WIDE(TS_FUNCTION_LOG_SIMPLE), TS_FMT(TS_WIDE(messageParam), ##__VA_ARGS__)/*, __FILE__, __LINE__*/); \
	} while(false)
	
	#define TS_WLOG_ERROR(messageParam, ...)   TS_WLOG_WITH_SEVERITY_IMPL(L"Error", messageParam, ##__VA_ARGS__)
	#define TS_WLOG_WARNING(messageParam, ...) TS_WLOG_WITH_SEVERITY_IMPL(L"Warning", messageParam, ##__VA_ARGS__)
	#define TS_WLOG_INFO(messageParam, ...)    TS_WLOG_WITH_SEVERITY_IMPL(L"Info", messageParam, ##__VA_ARGS__)

	#if TS_BUILD != TS_FINALRELEASE
		#define TS_WLOG_DEBUG(messageParam, ...) TS_WLOG_WITH_SEVERITY_IMPL(L"Debug", messageParam, ##__VA_ARGS__)
	#else
		#define TS_WLOG_DEBUG(messageParam, ...) ((void)0)
	#endif
	
#else
	
	#define TS_WLOG_ERROR(messageParam, ...)   TS_LOG_ERROR(messageParam, __VA_ARGS__)
	#define TS_WLOG_WARNING(messageParam, ...) TS_LOG_WARNING(messageParam, __VA_ARGS__)
	#define TS_WLOG_INFO(messageParam, ...)    TS_LOG_INFO(messageParam, __VA_ARGS__)

	#if TS_BUILD != TS_FINALRELEASE
		#define TS_WLOG_DEBUG(messageParam, ...) TS_LOG_DEBUG(messageParam, __VA_ARGS__)
	#else
		#define TS_WLOG_DEBUG(messageParam, ...) ((void)0)
	#endif
	
#endif

#include "ts/lang/common/Package.h"

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

