#pragma once

TS_PACKAGE1(thread)

class AbstractMutexBase : public lang::NoncopyableAndNonmovable
{
public:
	static const SizeType InvalidMutexOwner = ~0U;

	AbstractMutexBase() {}
	virtual ~AbstractMutexBase() {}

	virtual void lock() = 0;
	virtual bool tryLock() = 0;
	virtual void unlock() = 0;

protected:
	SizeType owner = InvalidMutexOwner;
};

TS_END_PACKAGE1()
