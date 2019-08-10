#include "Precompiled.h"
#include "ts/tessa/threading/Thread.h"

#include "ts/tessa/threading/BaseThreadEntry.h"
#include "ts/tessa/threading/ThreadUtils.h"

#include <atomic>

#if TS_PLATFORM == TS_WINDOWS
#include "ts/tessa/common/IncludeWindows.h"
#endif

TS_PACKAGE1(threading)

namespace
{
static const std::string mainThreadName = "MainThread";
static const std::string externalThreadName = "ExternalThread";
}

Thread Thread::firstThread(nullptr, mainThreadName);
Thread *Thread::mainThread = &firstThread;

Thread &Thread::getExternalThread()
{
	thread_local Thread externalThread(nullptr, externalThreadName);
	return externalThread;
}

thread_local Thread *Thread::currentThread = nullptr;

Thread *Thread::createThread(BaseThreadEntry *entryParam, const std::string &threadNameParam)
{
	Thread *thread = new Thread(entryParam, threadNameParam);
	return thread;
}

void Thread::joinThread(Thread *thread)
{
	TS_ASSERT(thread != nullptr);

	if (thread->threadImpl != nullptr)
	{
		thread->threadImpl->join();
		delete thread->threadImpl;
	}
	
	delete thread;
}

Thread &Thread::getMainThread()
{
	TS_ASSERT(Thread::mainThread != nullptr);
	return *Thread::mainThread;
}

Thread &Thread::getCurrentThread()
{
	if (Thread::currentThread == nullptr)
		Thread::currentThread = &getExternalThread();

	return *Thread::currentThread;
}

void Thread::setMainThread(Thread &thread)
{
	Thread::mainThread = &thread;
}

void Thread::sleep(SizeType milliseconds)
{
#if TS_PLATFORM == TS_WINDOWS
	Sleep(milliseconds);
#else
	std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
#endif
}

void Thread::startup()
{
	currentThread = this;
	running = true;
}

Thread::Thread(BaseThreadEntry *entryParam, const std::string &threadNameParam)
{
	entry = entryParam;
	threadName = threadNameParam;

	if (entry != nullptr)
	{
		entry->thread = this;

		threadImpl = new std::thread(&BaseThreadEntry::startup, entryParam);
		utils::setThreadName(*threadImpl, threadName);
	}
	else
	{
		// Main thread has no entry
		if (threadName == mainThreadName)
		{
			running = true;
			currentThread = this;
			utils::setCurrentThreadName(threadName);
		}
		else if (currentThread == nullptr)
		{
			currentThread = this;
			utils::setCurrentThreadName(threadName);
		}
	}
}

Thread::~Thread()
{
	if (this == mainThread)
	{
		running = false;
	}

	if (running != false && this != &firstThread)
	{
		TS_ASSERT(!"A thread is being destroyed while it's is still running!");
	}
}

TS_END_PACKAGE1()
