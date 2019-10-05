#include "Precompiled.h"
#include "CurrentThread.h"

#include "ts/thread/Thread.h"

#if TS_PLATFORM == TS_WINDOWS
	#include "ts/lang/common/IncludeWindows.h"
#endif

TS_PACKAGE1(thread)

const std::string &CurrentThread::getThreadName()
{
	return Thread::getCurrentThread().getThreadName();
}

SizeType CurrentThread::getThreadId()
{
	return Thread::getCurrentThread().getThreadId();
}

uint32 CurrentThread::getNativeThreadId()
{
#if TS_PLATFORM == TS_WINDOWS
	return GetCurrentThreadId();
#else
	static_assert(false, "Not implemented on this platform");
	return 0;
#endif
}

TS_END_PACKAGE1()
