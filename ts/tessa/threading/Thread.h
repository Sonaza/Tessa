#pragma once

#include <thread>
#include <atomic>

#include "ts/tessa/time/TimeSpan.h"

TS_DECLARE1(threading, BaseThreadEntry);

TS_PACKAGE1(threading)

class Thread
{
	friend class BaseThreadEntry;

public:
	static Thread *createThread(BaseThreadEntry *entry, const std::string &threadName);
	static void joinThread(Thread *thread);

// 	static std::string &getThreadName();

	static Thread &getMainThread();
	static Thread &getCurrentThread();

	static void setMainThread(Thread &thread);

	static void sleep(TimeSpan time);

	const std::string &getThreadName() const;
	const SizeType getThreadId() const;

private:
	static Thread &getExternalThread();

	void startup();

	bool running = false;
	SizeType threadId = 0;

	BaseThreadEntry *entry = nullptr;
	std::string threadName;

	std::thread *threadImpl = nullptr;

	static thread_local Thread *currentThread;

	static std::atomic<SizeType> nextThreadId;
	static Thread firstThread;
	static Thread *mainThread;

	// Use createThread and joinThread to handle creation and destruction
	Thread(BaseThreadEntry *entry, const std::string &threadName);
	~Thread();
};

TS_END_PACKAGE1()

TS_PACKAGE0()

using threading::Thread;

TS_END_PACKAGE0()