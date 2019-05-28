#include "Precompiled.h"
#include "ts/tessa/system/ThreadPool.h"

TS_PACKAGE1(system)

ThreadPool::ThreadPool(SizeType poolNumThreads)
{
	for (SizeType i = 0; i < poolNumThreads; ++i)
	{
		workerThreads.push_back(std::make_unique<std::thread>(
			&ThreadPool::threadTaskRunnerImpl, this, i
		));
	}
}

ThreadPool::~ThreadPool()
{
	{
		std::unique_lock<std::mutex> lock(queueMutex);
		running = false;
	}

	condition.notify_all();

	for (SizeType i = 0; i < workerThreads.size(); ++i)
	{
		workerThreads[i]->join();
	}
}

SizeType ThreadPool::numTasks() const
{
	std::unique_lock<std::mutex> lock(queueMutex);
	return (SizeType)taskQueue.size();
}

bool ThreadPool::hasTasks() const
{
	std::unique_lock<std::mutex> lock(queueMutex);
	return !taskQueue.empty();
}

void ThreadPool::clearTasks()
{
	{
		std::unique_lock<std::mutex> lock(queueMutex);
		TaskQueueType empty;
		std::swap(taskQueue, empty);
	}
	condition.notify_all();
}

void ThreadPool::threadTaskRunnerImpl(SizeType threadIndex)
{
	while (true)
	{
		TaskContainer task;

		{
			std::unique_lock<std::mutex> lock(queueMutex);
			condition.wait(lock, [this]()
			{
				return !running || !taskQueue.empty();
			});

			if (!running && taskQueue.empty())
				return;

			task = std::move(taskQueue.top());
			taskQueue.pop();

			TS_PRINTF("[Thread #%d] Task aqcuired, priority is %d\n", threadIndex, task.priority);
		}
		
		task.task();
	}
}

TS_END_PACKAGE1()
