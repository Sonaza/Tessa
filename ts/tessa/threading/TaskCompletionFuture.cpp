#include "Precompiled.h"
#include "TaskCompletionFuture.h"

TS_PACKAGE1(threading)

TaskCompletionFuture::TaskCompletionFuture(TaskCompletionPromise *promise)
	: promise(promise)
{
	TS_ASSERT(promise != nullptr);
	future = promise->promise.get_future();
}

TaskCompletionFuture::TaskCompletionFuture()
{
	TS_ASSERT(!"Don't call this");
}

TaskCompletionFuture::~TaskCompletionFuture()
{
// 	TS_PRINTF("Destroy TaskCompletionFuture!\n");
}

TaskCompletionFuture::TaskCompletionFuture(TaskCompletionFuture &&other)
{
	*this = std::move(other);
}

TaskCompletionFuture &TaskCompletionFuture::operator=(TaskCompletionFuture &&other)
{
	if (this != &other)
	{
		std::swap(promise, other.promise);
		future = std::move(other.future);
	}
	return *this;
}

void TaskCompletionFuture::waitForCompletion(SizeType taskId)
{
	TS_PRINTF("TaskCompletionFuture::waitForCompletion() Task ID %u\n", taskId);
	TS_ASSERT(future.valid());
	future.get();

	TS_PRINTF("All donezo!\n");
}

//////////////////////////////

TaskCompletionPromise::TaskCompletionPromise()
{

}

TaskCompletionPromise::TaskCompletionPromise(TaskCompletionPromise &&other)
{
	*this = std::move(other);
}

TaskCompletionPromise &TaskCompletionPromise::operator=(TaskCompletionPromise &&other)
{
	if (this != &other)
	{
		promise = std::move(other.promise);
// 		std::swap(completed, other.completed);
	}
	return *this;
}

TaskCompletionPromise::~TaskCompletionPromise()
{
}

void TaskCompletionPromise::signalCompletion()
{
// 	TS_ASSERT(!completed);
	promise.set_value();
// 	completed = true;
}

void TaskCompletionPromise::resetPromise()
{
	promise = std::promise<void>();
// 	completed = false;
}

TaskCompletionFuture TaskCompletionPromise::getFuture()
{
	return TaskCompletionFuture(this);
}

TS_END_PACKAGE1()