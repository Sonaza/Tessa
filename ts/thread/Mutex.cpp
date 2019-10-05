#include "Precompiled.h"
#include "Mutex.h"

#include "ts/thread/CurrentThread.h"
#include "ts/profiling/ZoneProfiler.h"

TS_PACKAGE1(thread)

Mutex::Mutex()
{

}

Mutex::~Mutex()
{
	TS_ASSERT(owner == InvalidMutexOwner && "Attempting destruction on a locked mutex.");
}

void Mutex::lock()
{
	TS_ZONE_MUTEX(owner, owner != InvalidMutexOwner);

	TS_ASSERT(owner != CurrentThread::getThreadId() && "Same thread trying to lock again.");

	mutex.lock();
	owner = CurrentThread::getThreadId();
}

bool Mutex::tryLock()
{
	TS_ASSERT(owner != CurrentThread::getThreadId() && "Same thread trying to lock again.");

	if (!mutex.try_lock())
		return false;

	owner = CurrentThread::getThreadId();

	return true;
}

void Mutex::unlock()
{
	TS_ASSERT(owner == CurrentThread::getThreadId() && "A thread that does not own the mutex is trying to unlock it.");

	owner = InvalidMutexOwner;
	mutex.unlock();
}

std::mutex &Mutex::getInternalMutex()
{
	return mutex;
}

TS_END_PACKAGE1()
