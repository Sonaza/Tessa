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

std::atomic<SizeType> Thread::nextThreadId;
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

void Thread::sleep(TimeSpan time)
{
	TS_ASSERT(time >= TimeSpan::zero && "Sleep time is negative.");

#if TS_PLATFORM == TS_WINDOWS
	Sleep((DWORD)time.getMilliseconds());
#else
	std::this_thread::sleep_for(std::chrono::milliseconds(time.getMilliseconds()));
#endif
}

const std::string &Thread::getThreadName() const
{
	return threadName;
}

const SizeType Thread::getThreadId() const
{
	return threadId;
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
	threadId = nextThreadId.fetch_add(1, std::memory_order_relaxed);

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
