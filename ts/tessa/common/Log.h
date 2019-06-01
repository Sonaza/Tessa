#pragma once

#include <cstdarg>

TS_PACKAGE1(log)

#if TS_BUILD != TS_FINALRELEASE
	#define TS_WPRINTF(format, ...) do { ::ts::log::printf(L ## format, __VA_ARGS__); } while(false)
	#define TS_PRINTF(format, ...)  do { ::ts::log::printf(format, __VA_ARGS__); } while(false)
#else
	#define TS_WPRINTF(...) ((void)0)
	#define TS_PRINTF(...)  ((void)0)
#endif

extern void initialize();
extern void finalize();

extern void printf(const char* format, ...);
extern void printf(const char* format, va_list args);

extern void printf(const wchar_t* format, ...);
extern void printf(const wchar_t* format, va_list args);

extern const std::string getTimestampString();
extern const std::wstring getTimestampStringWide();

TS_END_PACKAGE1()

