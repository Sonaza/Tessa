#pragma once

#include "ts/tessa/thread/AbstractMutexBase.h"

#include <mutex>

TS_PACKAGE1(thread)

class RecursiveMutex : public thread::AbstractMutexBase
{
public:
	RecursiveMutex();
	~RecursiveMutex();

	virtual void lock() override;
	virtual bool tryLock() override;
	virtual void unlock() override;

	std::mutex &getInternalMutex();

private:
	std::mutex mutex;
	SizeType numLocks = 0;
};

TS_END_PACKAGE1()

TS_PACKAGE0()

using thread::RecursiveMutex;

TS_END_PACKAGE0()