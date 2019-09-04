#include "Precompiled.h"
#include "Mutex.h"

#include "ts/tessa/thread/CurrentThread.h"
#include "ts/tessa/lang/CallStack.h"

#include "ts/tessa/profiling/ScopedZoneTimer.h"

#if TS_MUTEX_PROFILING == TS_TRUE
#pragma optimize("", off)
#endif

TS_PACKAGE1(thread)

Mutex::Mutex()
{

}

Mutex::~Mutex()
{
	TS_ASSERT(owner == InvalidOwner && "Attempting destruction on a locked mutex.");
}

void Mutex::lock(const char *debugInfo)
{
	TS_ZONE();

#if TS_MUTEX_PROFILING == TS_TRUE
	SizeType previousOwner = owner;
	std::string previousOwnerDebugInfo = ownerDebugInfo;
	TS_UNUSED_VARIABLE(previousOwner);
	TS_UNUSED_VARIABLE(previousOwnerDebugInfo);

	Time start = Time::now();
#endif

	TS_ASSERT(owner != CurrentThread::getThreadId() && "Same thread trying to lock again.");

	mutex.lock();
	owner = CurrentThread::getThreadId();

#if TS_MUTEX_PROFILING == TS_TRUE

	if (previousOwner != InvalidOwner)
	{
		TimeSpan blockedTime = (Time::now() - start);
		if (blockedTime.getMilliseconds() >= 10)
		{
			TS_PRINTF("Mutex locking was blocked for %s\n", blockedTime.getAsString());
		}

// 		if (blockedTime.getMilliseconds() >= 250)
// 		{
// 			TS_ASSERT(!"Mutex locking was blocked for over 250 milliseconds.");
// 		}
	}

	ownerDebugInfo = debugInfo ? std::string(debugInfo) : "no debug info";
#endif
}

bool Mutex::tryLock()
{
	if (!mutex.try_lock())
		return false;

	owner = CurrentThread::getThreadId();

	return true;
}

void Mutex::unlock()
{
#if TS_MUTEX_PROFILING == TS_TRUE
	ownerDebugInfo.clear();
#endif

	owner = InvalidOwner;
	mutex.unlock();
}

std::mutex &Mutex::getInternalMutex()
{
	return mutex;
}

TS_END_PACKAGE1()
