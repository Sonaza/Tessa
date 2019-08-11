#pragma once

#include <thread>

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

private:
	static Thread &getExternalThread();

	void startup();

	bool running = false;

	BaseThreadEntry *entry = nullptr;
	std::string threadName;

	std::thread *threadImpl = nullptr;

	static thread_local Thread *currentThread;

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