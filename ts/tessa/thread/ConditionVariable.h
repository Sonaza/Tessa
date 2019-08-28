#pragma once

#include "ts/tessa/thread/Mutex.h"
#include "ts/tessa/thread/MutexGuard.h"

#include <condition_variable>
#include <functional>

TS_PACKAGE1(thread)

class ConditionVariable : public lang::NoncopyableAndNonmovable
{
public:
	ConditionVariable();
	~ConditionVariable();

	void wait(MutexGuard &lock);
	void wait(MutexGuard &lock, const std::function<bool()> &predicate);

	// Returns false if wait period expired, otherwise true.
	bool waitFor(MutexGuard &lock, TimeSpan timeout);

	// Returns false if the predicate pred still evaluates to false
	// after the wait period expired, otherwise true.
	bool waitFor(MutexGuard &lock, TimeSpan timeout, const std::function<bool()> &predicate);

	void notifyOne();
	void notifyAll();

private:
// 	void lock(MutexGuard &lock);
// 	void unlock(MutexGuard &lock);

	std::condition_variable_any condition;
};

TS_END_PACKAGE1()

TS_PACKAGE0()

using thread::ConditionVariable;

TS_END_PACKAGE0()