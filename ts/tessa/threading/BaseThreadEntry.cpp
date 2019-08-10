#include "Precompiled.h"
#include "ts/tessa/threading/BaseThreadEntry.h"

#include "ts/tessa/threading/Thread.h"

TS_PACKAGE1(threading)

BaseThreadEntry::BaseThreadEntry()
{

}

BaseThreadEntry::~BaseThreadEntry()
{

}

void BaseThreadEntry::startup()
{
	TS_ASSERT(thread != nullptr);
	thread->startup();
	entry();
	thread->running = false;
}

TS_END_PACKAGE1()
