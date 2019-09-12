#pragma once

#include "ts/tessa/Config.h"

// Visual leak detector only works on MSVC in debug mode
#if defined(_MSC_VER) && TS_BUILD == TS_DEBUG && TS_DEBUG_MEMORY_LEAKS == TS_TRUE
	#include "vld.h"
#endif

TS_PACKAGE1(debugging)

extern bool isDebuggerPresent();

TS_END_PACKAGE1()
