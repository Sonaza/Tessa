#pragma once

#include "ts/thread/AbstractMutexBase.h"

TS_PACKAGE1(thread)

struct DeferLockType { explicit DeferLockType() = default; };
struct TryToLockType { explicit TryToLockType() = default; };
struct AdoptLockType { explicit AdoptLockType() = default; };

constexpr DeferLockType DeferLock{};
constexpr TryToLockType TryToLock{};
constexpr AdoptLockType AdoptLock{};

class MutexGuard : public lang::Noncopyable
{
public:
	// Default option, immediately attempts lock and blocks until able.
	MutexGuard(AbstractMutexBase &mutex);

	// Defers locking, not attempting a lock on construct.
	MutexGuard(AbstractMutexBase &mutex, DeferLockType t);

	// Tries to lock but continues unblocked if not able.
	MutexGuard(AbstractMutexBase &mutex, TryToLockType t);

	// Adopts a mutex that is already locked. Error if mutex is not locked.
	MutexGuard(AbstractMutexBase &mutex, AdoptLockType t);

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