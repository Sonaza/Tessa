#pragma once

#include "ts/thread/AbstractMutexBase.h"

#include <mutex>

TS_PACKAGE1(thread)

class Mutex : public thread::AbstractMutexBase
{
public:
	Mutex();
	~Mutex();

	virtual void lock() override;
	virtual bool tryLock() override;
	virtual void unlock() override;

	std::mutex &getInternalMutex();

private:
	std::mutex mutex;
};

TS_END_PACKAGE1()

TS_PACKAGE0()

using thread::Mutex;

TS_END_PACKAGE0()