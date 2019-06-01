#pragma once

namespace ts
{

void __cdecl breakpoint();
void __cdecl assert_impl(const char* message, const char* filepath, const unsigned int line);

}

#define __TS_STRINGIFY_(str) (# str)
#define __TS_STRINGIFY(str)	__TS_STRINGIFY_(str)

#define __TS_ASSERT_IMPL(_assertion)			(void)( (!!(_assertion)) || (::ts::assert_impl(__TS_STRINGIFY(_assertion), __FILE__, __LINE__), 0) )
#define __TS_ASSERTF_IMPL(_assertion, _message)	(void)( (!!(_assertion)) || (::ts::assert_impl(_message, __FILE__, __LINE__), 0) )

#if TS_BUILD == TS_DEBUG

	#define TS_ASSERT(assertion)						__TS_ASSERT_IMPL(assertion)
	#define TS_EXPENSIVE_ASSERT(assertion)				__TS_ASSERT_IMPL(assertion)
	#define TS_ASSERTF(assertion, message)				__TS_ASSERTF_IMPL(assertion, message)
	#define TS_EXPENSIVE_ASSERTF(assertion, message)	__TS_ASSERTF_IMPL(assertion, message)

#elif TS_BUILD == TS_RELEASE

	#define TS_ASSERT(assertion)						__TS_ASSERT_IMPL(assertion)
	#define TS_EXPENSIVE_ASSERT(assertion)				((void)0)
	#define TS_ASSERTF(assertion, message)				__TS_ASSERTF_IMPL(assertion, message)
	#define TS_EXPENSIVE_ASSERTF(assertion, message)	((void)0)

#elif TS_BUILD == TS_FINALRELEASE

	#define TS_ASSERT(assertion)						((void)0)
	#define TS_EXPENSIVE_ASSERT(assertion)				((void)0)
	#define TS_ASSERTF(assertion, message)				((void)0)
	#define TS_EXPENSIVE_ASSERTF(assertion, message)	((void)0)

#endif
