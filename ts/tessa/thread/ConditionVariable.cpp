#include "Precompiled.h"
#include "ConditionVariable.h"

#include "ts/tessa/thread/CurrentThread.h"

TS_PACKAGE1(thread)

ConditionVariable::ConditionVariable()
{

}

ConditionVariable::~ConditionVariable()
{

}

void ConditionVariable::wait(MutexGuard &lock)
{
	TS_ASSERT(lock.isLocked() && "MutexGuard must be locked before entering wait.");
	condition.wait(lock);
}

void ConditionVariable::wait(MutexGuard &lock, const std::function<bool()> &predicate)
{
	TS_ASSERT(lock.isLocked() && "MutexGuard must be locked before entering wait.");
	condition.wait(lock, std::move(predicate));
}

bool ConditionVariable::waitFor(MutexGuard &lock, TimeSpan timeout)
{
	TS_ASSERT(lock.isLocked() && "MutexGuard must be locked before entering wait.");
	return condition.wait_for(
		lock,
		std::chrono::microseconds(timeout.getMicroseconds())) == std::cv_status::no_timeout;
}

bool ConditionVariable::waitFor(MutexGuard &lock, TimeSpan timeout, const std::function<bool()> &predicate)
{
	TS_ASSERT(lock.isLocked() && "MutexGuard must be locked before entering wait.");
	return condition.wait_for(
		lock,
		std::chrono::microseconds(timeout.getMicroseconds()),
		std::move(predicate));
}

void ConditionVariable::notifyOne()
{
	condition.notify_one();
}

void ConditionVariable::notifyAll()
{
	condition.notify_all();
}

// void ConditionVariable::lock(MutexGuard &lock)
// {
// 	lock.ownsLock = true;
// 	lock.mutex->owner = CurrentThread::getThreadId();
// }
// 
// void ConditionVariable::unlock(MutexGuard &lock)
// {
// 	lock.ownsLock = false;
// 	lock.mutex->owner = Mutex::InvalidOwner;
// }

TS_END_PACKAGE1()
