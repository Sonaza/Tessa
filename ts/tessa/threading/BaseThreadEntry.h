#pragma once

#include <thread>
#include <condition_variable>
#include <functional>
#include <memory>
#include <queue>
#include <future>

TS_PACKAGE1(threading)

class BaseThreadEntry
{
	friend class Thread;

public:
	BaseThreadEntry();
	virtual ~BaseThreadEntry();

	virtual void entry() = 0;

protected:
	void startup();

	Thread *thread;
};

TS_END_PACKAGE1()
