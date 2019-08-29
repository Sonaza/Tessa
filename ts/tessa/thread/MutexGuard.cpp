#include "Precompiled.h"
#include "MutexGuard.h"

TS_PACKAGE1(thread)

MutexGuard::MutexGuard(Mutex &mutexParam, const char *debugInfo)
	: mutex(std::addressof(mutexParam))
{
	mutex->lock(debugInfo);
	ownsLock = true;
#if TS_MUTEX_PROFILING == TS_TRUE
	savedDebugInfo = debugInfo;
#endif
}

MutexGuard::MutexGuard(MutexGuard &&other)
{
	*this = std::move(other);
}

MutexGuard &MutexGuard::operator=(MutexGuard &&other)
{
	if (this != &other)
	{
		std::swap(mutex, other.mutex);
		ownsLock = other.ownsLock.load();
		other.ownsLock = false;
	}
	return *this;
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
#if TS_MUTEX_PROFILING == TS_TRUE
	mutex->lock(savedDebugInfo);
#else
	mutex->lock();
#endif
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
