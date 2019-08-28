#include "Precompiled.h"
#include "ts/tessa/thread/AbstractThreadEntry.h"

#include "ts/tessa/thread/Thread.h"

TS_PACKAGE1(thread)

AbstractThreadEntry::AbstractThreadEntry()
{

}

AbstractThreadEntry::~AbstractThreadEntry()
{

}

void AbstractThreadEntry::startup()
{
	TS_ASSERT(thread != nullptr);
	thread->startup();
	entry();
	thread->running = false;
}

TS_END_PACKAGE1()
