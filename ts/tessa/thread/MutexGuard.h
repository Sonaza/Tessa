#pragma once

#include "ts/tessa/thread/Mutex.h"

#define TS_EXPAND_1(x) #x
#define TS_EXPAND(x)   TS_EXPAND_1(x)
#define MUTEXGUARD_DEBUGINFO() TS_FUNCTION_LOG_SIMPLE "  " TS_EXPAND(__FILE__) " : " TS_EXPAND(__LINE__)

TS_PACKAGE1(thread)

class MutexGuard : public lang::NoncopyableAndNonmovable
{
	friend class ConditionVariable;

public:
	MutexGuard(Mutex &mutex, const char *debugInfo = nullptr);
	~MutexGuard();

	void lock();
	void unlock();

	bool isLocked() const;

private:
	Mutex *mutex = nullptr;
	std::atomic_bool ownsLock;
};

TS_END_PACKAGE1()

TS_PACKAGE0()

using thread::MutexGuard;

TS_END_PACKAGE0()