#pragma once

#include <thread>
#include <condition_variable>
#include <mutex>

TS_PACKAGE1(threading)

class TaskCompletionPromise;

class TaskCompletionFuture
{
	friend class TaskCompletionPromise;
public:
	TaskCompletionFuture();

	void waitForCompletion();

private:
	TaskCompletionFuture(TaskCompletionPromise *promise);

	TaskCompletionPromise *promise = nullptr;
};

class TaskCompletionPromise
{
	friend class TaskCompletionFuture;
public:
	TaskCompletionPromise() = default;

	void signalCompletion();
	void reset();
	TaskCompletionFuture getFuture();

private:
	std::mutex mutex;
	std::condition_variable condition;
	bool isComplete = false;
};

TS_END_PACKAGE1()
