#include "Precompiled.h"
#include "AbstractMutexBase.h"

#include "ts/thread/CurrentThread.h"

TS_PACKAGE1(thread)

AbstractMutexBase::AbstractMutexBase()
{

}

AbstractMutexBase::~AbstractMutexBase()
{

}

bool AbstractMutexBase::hasOwner() const
{
	return owner != InvalidMutexOwner;
}

bool AbstractMutexBase::ownerIsCurrentThread() const
{
	return owner == CurrentThread::getThreadId();
}

TS_END_PACKAGE1()
