#pragma once

#include "ts/tessa/thread/Mutex.h"

#define TS_EXPAND_1(x) #x
#define TS_EXPAND(x)   TS_EXPAND_1(x)
#define MUTEXGUARD_DEBUGINFO() TS_FUNCTION_LOG_SIMPLE "  " TS_EXPAND(__FILE__) " : " TS_EXPAND(__LINE__)

TS_PACKAGE1(thread)

class MutexGuard : public lang::Noncopyable
{
public:
	MutexGuard(Mutex &mutex, const char *debugInfo = nullptr);
	~MutexGuard();

	MutexGuard(MutexGuard &&other);
	MutexGuard &operator=(MutexGuard &&other);

	void lock();
	void unlock();

	bool isLocked() const;

private:
	Mutex *mutex = nullptr;
	std::atomic_bool ownsLock;

#if TS_MUTEX_PROFILING == TS_TRUE
	const char *savedDebugInfo = nullptr;
#endif
};

TS_END_PACKAGE1()

TS_PACKAGE0()

using thread::MutexGuard;

TS_END_PACKAGE0()