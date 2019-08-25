#include "Precompiled.h"
#include "TaskCompletionFuture.h"

TS_PACKAGE1(threading)

void TaskCompletionFuture::waitForCompletion()
{
	TS_ASSERT(promise != nullptr);
	std::unique_lock<std::mutex> lock(promise->mutex);

	if (promise->isComplete)
	{
		TS_PRINTF("Already complete, skedaddlering...");
		return;
	}
	TS_PRINTF("Not yet complete. Starting to wait...\n");

	promise->condition.wait(lock, [this]()
	{
		return promise->isComplete;
	});

	TS_PRINTF("All donezo!\n");
}

TaskCompletionFuture::TaskCompletionFuture(TaskCompletionPromise *promise)
	: promise(promise)
{
	TS_ASSERT(promise != nullptr);
}

TaskCompletionFuture::TaskCompletionFuture()
{
	TS_ASSERT(!"Don't call this");
}

//////////////////////////////

void TaskCompletionPromise::signalCompletion()
{
	{
		std::unique_lock<std::mutex> lock(mutex);
		TS_ASSERT(isComplete == false && "TaskCompletionPromise completion was already signaled.");
		isComplete = true;
	}
	condition.notify_all();
}

void TaskCompletionPromise::resetPromise()
{
	{
		std::unique_lock<std::mutex> lock(mutex);
		isComplete = false;
	}
}

TaskCompletionFuture TaskCompletionPromise::getFuture()
{
	return TaskCompletionFuture(this);
}

TS_END_PACKAGE1()