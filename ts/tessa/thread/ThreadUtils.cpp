#include "Precompiled.h"
#include "ts/tessa/thread/ThreadUtils.h"

#if TS_PLATFORM == TS_WINDOWS
#include "ts/tessa/common/IncludeWindows.h"
#include <processthreadsapi.h>
#pragma warning( disable : 4702 ) // Unreachable code warning, it's just the NYI asserts.
#endif

TS_PACKAGE2(thread, utils)

namespace
{

int32 convertToSystemThreadPriority(ThreadPriority priority)
{
#if TS_PLATFORM == TS_WINDOWS
	switch(priority)
	{
		case ThreadPriority_Critical:    return THREAD_PRIORITY_TIME_CRITICAL;
		case ThreadPriority_High:        return THREAD_PRIORITY_HIGHEST;
		case ThreadPriority_AboveNormal: return THREAD_PRIORITY_ABOVE_NORMAL;
		case ThreadPriority_Normal:      return THREAD_PRIORITY_NORMAL;
		case ThreadPriority_BelowNormal: return THREAD_PRIORITY_BELOW_NORMAL;
		case ThreadPriority_Low:         return THREAD_PRIORITY_LOWEST;
		case ThreadPriority_Idle:        return THREAD_PRIORITY_IDLE;
	}
#endif
	TS_ASSERT(!"Not implemented on this platform.");
	return 0;
}

#if TS_PLATFORM == TS_WINDOWS

void setThreadName(DWORD nativeThreadID, const std::string &threadName)
{
	/*
	Kinda terrible but Microsoft gonna Microsoft
	https://docs.microsoft.com/en-us/visualstudio/debugger/how-to-set-a-thread-name-in-native-code

	Newer alternative for Win10 on VS2017
	https://docs.microsoft.com/en-us/windows/desktop/api/processthreadsapi/nf-processthreadsapi-setthreaddescription
	*/

	struct TS_ALIGN(8)
	{
		DWORD dwType;     // Must be 0x1000.
		LPCSTR szName;    // Pointer to name (in user addr space).
		DWORD dwThreadID; // Thread ID (-1=caller thread).
		DWORD dwFlags;    // Reserved for future use, must be zero.
	} info;
	info.dwType = 0x1000;
	info.szName = threadName.c_str();
	info.dwThreadID = nativeThreadID;;
	info.dwFlags = 0;

#pragma warning( push )
#pragma warning( disable : 6320 6322 ) // Mute warnings about catch-all exception handling and empty block
	__try
	{
		const DWORD MS_VC_THREAD_NAMING_EXCEPTION = 0x406D1388;
		RaiseException(MS_VC_THREAD_NAMING_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
#pragma warning( pop )
	return;
}

#endif


}

extern void setThreadName(std::thread &thread, const std::string &threadName)
{
#if TS_PLATFORM == TS_WINDOWS
	setThreadName(GetThreadId(thread.native_handle()), threadName);
	return;
#endif

	TS_ASSERT(!"Not implemented on this platform.");
}

extern void setCurrentThreadName(const std::string &threadName)
{
#if TS_PLATFORM == TS_WINDOWS
	setThreadName(GetCurrentThreadId(), threadName);
	return;
#endif

	TS_ASSERT(!"Not implemented on this platform.");
}

extern void setThreadPriority(std::thread &thread, ThreadPriority priority)
{
#if TS_PLATFORM == TS_WINDOWS
	SetThreadPriority(thread.native_handle(), convertToSystemThreadPriority(priority));
	return;
#endif

	TS_ASSERT(!"Not implemented on this platform.");
}

extern void setCurrentThreadPriority(ThreadPriority priority)
{
#if TS_PLATFORM == TS_WINDOWS
	SetThreadPriority(GetCurrentThread(), convertToSystemThreadPriority(priority));
	return;
#endif

	TS_ASSERT(!"Not implemented on this platform.");
}

TS_END_PACKAGE2()
