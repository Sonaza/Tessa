#include "Precompiled.h"
#include "RecursiveMutex.h"

#include "ts/thread/CurrentThread.h"
#include "ts/profiling/ZoneProfiler.h"

TS_PACKAGE1(thread)

RecursiveMutex::RecursiveMutex()
{

}

RecursiveMutex::~RecursiveMutex()
{
	TS_ASSERT(owner == InvalidMutexOwner && "Attempting destruction on a locked mutex.");
}

void RecursiveMutex::lock()
{
	if (owner == CurrentThread::getThreadId())
	{
		numLocks++;
		return;
	}

	TS_ZONE_MUTEX(owner, owner != InvalidMutexOwner);

	mutex.lock();
	owner = CurrentThread::getThreadId();
}

bool RecursiveMutex::tryLock()
{
	if (owner == CurrentThread::getThreadId())
	{
		numLocks++;
		return true;
	}
	else if (!mutex.try_lock())
		return false;

	owner = CurrentThread::getThreadId();

	return true;
}

void RecursiveMutex::unlock()
{
	TS_ASSERT(owner == CurrentThread::getThreadId() && "A thread that does not own the mutex is trying to unlock it.");

	if (numLocks > 0)
	{
		numLocks--;
		return;
	}

	owner = InvalidMutexOwner;
	mutex.unlock();
}

std::mutex &RecursiveMutex::getInternalMutex()
{
	return mutex;
}

TS_END_PACKAGE1()
