#pragma once

#if TS_PLATFORM == TS_WINDOWS
	#define __TS_DEBUG_BREAK  __debugbreak()
#elif TS_PLATFORM == TS_LINUX
	#define __TS_DEBUG_BREAK  __builtin_trap()
#elif
	#error "Unsupported platform."
#endif

#define __TS_ASSERT_IMPL_IMPL(__expression_str, __message_str) \
	::ts::assert_impl(__expression_str, __message_str, __FILE__, __LINE__)

#define __TS_ASSERT_IMPL(__expression) \
	do { \
		if (!(__expression)) { \
			if (__TS_ASSERT_IMPL_IMPL(TS_STRINGIFY(__expression), nullptr)) __TS_DEBUG_BREAK; \
		} \
	} while(false)

#define __TS_ASSERTF_IMPL(__expression, __message, ...) \
	do { \
		if (!(__expression)) { \
			const std::string &__fmt_message = TS_FMT(__message, ##__VA_ARGS__); \
			if (__TS_ASSERT_IMPL_IMPL(TS_STRINGIFY(__expression), __fmt_message.c_str())) __TS_DEBUG_BREAK; \
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
	#define TS_ASSERTF(assertion, message)                  ((void)0)
	#define TS_EXPENSIVE_ASSERTF(assertion, message)        ((void)0)


#endif

namespace ts
{

bool __cdecl assert_impl(const char *expression, const char *message, const char *filepath, const unsigned int line);

}