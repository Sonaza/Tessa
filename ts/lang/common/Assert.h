#pragma once

// #include <intrin.h>

#if TS_COMPILER == TS_MSC
	#define TS_DEBUG_BREAK  __debugbreak()
#elif TS_COMPILER == TS_GNUC || TS_COMPILER == TS_CLANG
	#define TS_DEBUG_BREAK  __builtin_trap()
#else
	#error "Unsupported platform."
#endif

#define __TS_STRINGIFY(str) (# str)
#define TS_STRINGIFY(str)   __TS_STRINGIFY(str)

#define __TS_ASSERT_IMPL_IMPL(__expression_str, __message_str) \
	::ts::_assert_impl(__expression_str, __message_str, __FILE__, __LINE__)

#define __TS_ASSERT_IMPL(__expression) \
	do { \
		if (!(__expression)) { \
			if (__TS_ASSERT_IMPL_IMPL(TS_STRINGIFY(__expression), "")) TS_DEBUG_BREAK; \
		} \
	} while(false)

#define __TS_ASSERTF_IMPL(__expression, __message, ...) \
	do { \
		if (!(__expression)) { \
			String __fmt_message; __fmt_message.sprintf(__message, ##__VA_ARGS__); \
			if (__TS_ASSERT_IMPL_IMPL(TS_STRINGIFY(__expression), __fmt_message)) TS_DEBUG_BREAK; \
		} \
	} while(false)

#if TS_BUILD == TS_DEBUG

	#define TS_ASSERT(assertion)                            __TS_ASSERT_IMPL(assertion)
	#define TS_EXPENSIVE_ASSERT(assertion)                  __TS_ASSERT_IMPL(assertion)
	#define TS_ASSERTF(assertion, message, ...)             __TS_ASSERTF_IMPL(assertion, message, ##__VA_ARGS__)
	#define TS_EXPENSIVE_ASSERTF(assertion, message, ...)   __TS_ASSERTF_IMPL(assertion, message, ##__VA_ARGS__)

#elif TS_BUILD == TS_RELEASE

	#define TS_ASSERT(assertion)                            __TS_ASSERT_IMPL(assertion)
	#define TS_EXPENSIVE_ASSERT(assertion)                  ((void)0)
	#define TS_ASSERTF(assertion, message, ...)             __TS_ASSERTF_IMPL(assertion, message, ##__VA_ARGS__)
	#define TS_EXPENSIVE_ASSERTF(assertion, message, ...)   ((void)0)

#elif TS_BUILD == TS_FINALRELEASE

	#define TS_ASSERT(assertion)                            ((void)0)
	#define TS_EXPENSIVE_ASSERT(assertion)                  ((void)0)
	#define TS_ASSERTF(assertion, message, ...)             ((void)0)
	#define TS_EXPENSIVE_ASSERTF(assertion, message, ...)   ((void)0)

#endif

#include "ts/lang/common/Package.h"

TS_DECLARE1(string, String);

TS_PACKAGE0()

extern bool _assert_impl(
	const string::String &expression,
	const string::String &message,
	const string::String &filepath,
	const unsigned int line);

TS_END_PACKAGE0()
