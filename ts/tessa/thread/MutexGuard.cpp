#include "Precompiled.h"
#include "MutexGuard.h"

TS_PACKAGE1(thread)

MutexGuard::MutexGuard(AbstractMutexBase &mutexParam)
	: mutex(std::addressof(mutexParam))
{
	mutex->lock();
	ownsLock = true;
}

MutexGuard::MutexGuard(AbstractMutexBase &mutexParam, DeferLockType t)
	: mutex(std::addressof(mutexParam))
{
	// No locking here.
}

MutexGuard::MutexGuard(AbstractMutexBase &mutexParam, TryToLockType t)
	: mutex(std::addressof(mutexParam))
{
	if (mutex->tryLock())
		ownsLock = true;
}

MutexGuard::MutexGuard(AbstractMutexBase &mutexParam, AdoptLockType t)
	: mutex(std::addressof(mutexParam))
{
	TS_ASSERTF(mutexParam.ownerIsCurrentThread(), "Current thread is not the owner of the lock.");
	ownsLock = true;
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
	mutex->lock();
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
