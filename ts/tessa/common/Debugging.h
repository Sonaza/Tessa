#pragma once

#include "ts/tessa/Config.h"

// Visual leak detector only works on MSVC in debug mode
#if defined(_MSC_VER) && TS_BUILD == TS_DEBUG && TS_DEBUG_MEMORY_LEAKS == TS_TRUE
	#include "vld.h"
// 	#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.DTfW.DHL' version='6.11.1.404' processorArchitecture='amd64' language='*'\"")
#endif
