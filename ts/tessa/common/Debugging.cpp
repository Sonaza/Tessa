#include "Precompiled.h"
#include "Debugging.h"

#if TS_PLATFORM == TS_WINDOWS
	#include "ts/tessa/common/IncludeWindows.h"
#endif

TS_PACKAGE1(debugging)

extern bool isDebuggerPresent()
{
#if TS_PLATFORM == TS_WINDOWS
	return IsDebuggerPresent() == TRUE;
#else
	return false;
#endif
}

TS_END_PACKAGE1()


