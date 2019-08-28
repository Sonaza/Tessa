#pragma once

#include <thread>
#include <atomic>

#include "ts/tessa/thread/CurrentThread.h"

#include "ts/tessa/time/TimeSpan.h"

TS_DECLARE1(thread, AbstractThreadEntry);

TS_PACKAGE1(thread)

class Thread
{
	friend class AbstractThreadEntry;

public:
	static Thread *createThread(AbstractThreadEntry *entry, const std::string &threadName);
	static void joinThread(Thread *thread);

// 	static std::string &getThreadName();

	static Thread &getMainThread();
	static Thread &getCurrentThread();

	static void setMainThread(Thread &thread);

	static void sleep(TimeSpan time);

	const std::string &getThreadName() const;
	SizeType getThreadId() const;

private:
	static Thread &getExternalThread();

	static thread_local Thread *currentThread;

	static std::atomic<SizeType> nextThreadId;
	static Thread firstThread;
	static Thread *mainThread;

	void startup();

	bool running = false;
	SizeType threadId = 0;

	AbstractThreadEntry *entry = nullptr;
	std::string threadName;

	std::thread *threadImpl = nullptr;

	// Use createThread and joinThread to handle creation and destruction
	Thread(AbstractThreadEntry *entry, const std::string &threadName);
	~Thread();
};

TS_END_PACKAGE1()

TS_PACKAGE0()

using thread::Thread;

TS_END_PACKAGE0()