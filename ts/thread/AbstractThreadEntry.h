#pragma once

#include <thread>
#include <condition_variable>
#include <functional>
#include <memory>
#include <queue>
#include <future>

TS_DECLARE1(thread, Thread)

TS_PACKAGE1(thread)

class AbstractThreadEntry
{
	friend class Thread;

public:
	AbstractThreadEntry();
	virtual ~AbstractThreadEntry();

	virtual void entry() = 0;

	virtual String getDebugString() const { return ""; }

protected:
	void startup();

	Thread *thread;
};

TS_END_PACKAGE1()
