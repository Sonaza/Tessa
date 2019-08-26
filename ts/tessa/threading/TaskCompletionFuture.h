#pragma once

#include <thread>
#include <condition_variable>
#include <atomic>
#include <mutex>
#include <future>

TS_PACKAGE1(threading)

class TaskCompletionPromise;

class TaskCompletionFuture
{
	friend class TaskCompletionPromise;
public:
	TaskCompletionFuture();
	~TaskCompletionFuture();

	TaskCompletionFuture(TaskCompletionFuture &&other);
	TaskCompletionFuture &operator=(TaskCompletionFuture &&other);

	void waitForCompletion(SizeType taskId);

private:
	TaskCompletionFuture(TaskCompletionPromise *promise);

	TaskCompletionPromise *promise = nullptr;
	std::future<void> future;
};

class TaskCompletionPromise : lang::Noncopyable
{
	friend class TaskCompletionFuture;
public:
	TaskCompletionPromise();
	~TaskCompletionPromise();

	TaskCompletionPromise(TaskCompletionPromise &&other);
	TaskCompletionPromise &operator=(TaskCompletionPromise &&other);

	void signalCompletion();
	
	// Resetting promise requires retrieving new future
	void resetPromise();

	TaskCompletionFuture getFuture();

private:
	std::promise<void> promise;
// 	bool completed = false;
};

TS_END_PACKAGE1()
