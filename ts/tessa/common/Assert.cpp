#include "Precompiled.h"

#if TS_PLATFORM == TS_WINDOWS
	#include "ts/tessa/common/IncludeWindows.h"
#endif

TS_PACKAGE0()

void __cdecl breakpoint()
{
	#if TS_PLATFORM == TS_WINDOWS
		DebugBreak();
	#elif TS_PLATFORM == TS_LINUX
		__builtin_trap();
	#elif
		#error "Unsupported platform."
	#endif
}

void __cdecl assert_impl(const char* _assertion, const char* _file, const unsigned int _line)
{
	TS_PRINTF("Assertion failed: %s at %s:%u\n", _assertion, _file, _line);
	breakpoint();
}

TS_END_PACKAGE0()
