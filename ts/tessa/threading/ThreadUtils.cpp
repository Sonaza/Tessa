#include "Precompiled.h"
#include "ts/tessa/threading/ThreadUtils.h"

#if TS_PLATFORM == TS_WINDOWS
#include "ts/tessa/common/IncludeWindows.h"
#include <processthreadsapi.h>

#pragma warning( disable : 4702 ) // Unreachable code warning, it's just the NYI asserts.

__declspec(align(8)) struct THREADNAME_INFO
{
	DWORD dwType; // Must be 0x1000.
	LPCSTR szName; // Pointer to name (in user addr space).
	DWORD dwThreadID; // Thread ID (-1=caller thread).
	DWORD dwFlags; // Reserved for future use, must be zero.
};
#endif

TS_PACKAGE2(threading, utils)

namespace
{

Int32 convertToSystemThreadPriority(ThreadPriority priority)
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
	return 0;
}

}

extern void setThreadName(std::thread &thread, const std::string &threadName)
{
#if TS_PLATFORM == TS_WINDOWS

	DWORD threadID = GetThreadId(thread.native_handle());

	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = threadName.c_str();
	info.dwThreadID = threadID;
	info.dwFlags = 0;

#pragma warning( push )
#pragma warning( disable : 6320 6322 )
	static const DWORD MS_VC_THREAD_NAMING_EXCEPTION = 0x406D1388;
	__try
	{
		RaiseException(MS_VC_THREAD_NAMING_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
#pragma warning( pop )
	return;
#endif

	TS_ASSERT(!"Not implemented on this platform.");
}

extern void setThreadPriority(std::thread &thread, ThreadPriority priority)
{
#if TS_PLATFORM == TS_WINDOWS
	HANDLE threadHwnd = thread.native_handle();
	SetThreadPriority(threadHwnd, convertToSystemThreadPriority(priority));
	return;
#endif

	TS_ASSERT(!"Not implemented on this platform.");
}

TS_END_PACKAGE2()
