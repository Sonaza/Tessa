#pragma once

#include <mutex>

#define TS_MUTEX_PROFILING TS_TRUE

#if TS_MUTEX_PROFILING == TS_TRUE
	#include "ts/tessa/time/Time.h"
	#include "ts/tessa/time/TimeSpan.h"
#endif

TS_PACKAGE1(thread)

class Mutex : public lang::NoncopyableAndNonmovable
{
	friend class ConditionVariable;

public:
	Mutex();
	~Mutex();

	void lock(const char *debugInfo = nullptr);
	bool tryLock();

	void unlock();

	std::mutex &getInternalMutex();

private:
	std::mutex mutex;

	static const SizeType InvalidOwner = ~0U;
	SizeType owner = InvalidOwner;

#if TS_MUTEX_PROFILING == TS_TRUE
	std::string ownerDebugInfo;
#endif
};

TS_END_PACKAGE1()

TS_PACKAGE0()

using thread::Mutex;

TS_END_PACKAGE0()