#include "Precompiled.h"
#include "MutexGuard.h"

TS_PACKAGE1(thread)

MutexGuard::MutexGuard(Mutex &mutexParam, const char *debugInfo)
	: mutex(std::addressof(mutexParam))
{
	mutex->lock(debugInfo);
	ownsLock = true;
}

MutexGuard::~MutexGuard()
{
	if (ownsLock)
	{
		ownsLock = false;
		mutex->unlock();
	}
}

void MutexGuard::lock()
{
	mutex->lock("was manually locked");
	ownsLock = true;
}

void MutexGuard::unlock()
{
	TS_ASSERT(ownsLock == true && "Trying to unlock MutexGuard that does not own its mutex->");
	ownsLock = false;
	mutex->unlock();
}

bool MutexGuard::isLocked() const
{
	return ownsLock.load();
}

TS_END_PACKAGE1()
