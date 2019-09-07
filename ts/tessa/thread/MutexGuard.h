#pragma once

#include "ts/tessa/thread/AbstractMutexBase.h"

TS_PACKAGE1(thread)

class MutexGuard : public lang::Noncopyable
{
public:
	MutexGuard(AbstractMutexBase &mutex);
	~MutexGuard();

	MutexGuard(MutexGuard &&other);
	MutexGuard &operator=(MutexGuard &&other);

	void lock();
	void unlock();

	bool isLocked() const;

private:
	AbstractMutexBase *mutex = nullptr;
	std::atomic_bool ownsLock;
};

TS_END_PACKAGE1()

TS_PACKAGE0()

using thread::MutexGuard;

TS_END_PACKAGE0()