#include "Precompiled.h"
#include "ThreadScheduler.h"

#include "ts/tessa/threading/BaseThreadEntry.h"
#include "ts/tessa/threading/Thread.h"
#include "ts/tessa/threading/ThreadUtils.h"

#include "ts/tessa/Config.h"

#include <type_traits>

#pragma optimize("", off)

TS_DEFINE_MANAGER_TYPE(threading::ThreadScheduler);

TS_PACKAGE1(threading)

class ThreadScheduler::BackgroundScheduler : public BaseThreadEntry
{
	ThreadScheduler *scheduler = nullptr;

	Thread *thread;

public:
	BackgroundScheduler(ThreadScheduler *scheduler)
		: scheduler(scheduler)
	{
		thread = Thread::createThread(this, "ThreadScheduler::BackgroundScheduler");
	}

	~BackgroundScheduler()
	{
		if (thread != nullptr)
			Thread::joinThread(thread);
	}

	void entry()
	{
		while (true)
		{
			std::unique_lock<std::mutex> lock(scheduler->queueMutex);
			scheduler->schedulerCondition.wait_for(lock, std::chrono::milliseconds(10),
			[this]()
			{
				return !scheduler->running || !scheduler->waitingTaskQueue.empty();
			});

			if (!scheduler->running)// && scheduler->pendingTaskQueue.empty())
				return;

			if (scheduler->waitingTaskQueue.empty())
				continue;

			if (Time::now() >= scheduler->waitingTaskQueue.top().scheduledTime)
			{
				// Move task from pending list to the active list
				scheduler->pendingTaskQueue.push(std::move(scheduler->waitingTaskQueue.top()));
				scheduler->waitingTaskQueue.pop();

				lock.unlock();

				// Notifies any waiting worker to start handling it
				scheduler->workerCondition.notify_one();
			}
		}
	}
};

class ThreadScheduler::BackgroundWorker : public BaseThreadEntry
{
	ThreadScheduler *scheduler = nullptr;
	SizeType workerIndex = 0;
	SharedPointer<std::mutex> mutex;

	Thread *thread;

public:
	BackgroundWorker(ThreadScheduler *scheduler, SizeType workerIndex, SharedPointer<std::mutex> mutex)
		: scheduler(scheduler)
		, workerIndex(workerIndex)
		, mutex(mutex)
	{
		thread = Thread::createThread(this, TS_FMT("ThreadScheduler::BackgroundWorker %u", workerIndex));
	}

	~BackgroundWorker()
	{
		if (thread != nullptr)
			Thread::joinThread(thread);
	}

	void entry()
	{
		TS_LOG_DEBUG("BackgroundWorker %u running. Waiting for tasks...", workerIndex);

		while (true)
		{
			ScheduledTask task;
			{
				std::unique_lock<std::mutex> lock(scheduler->queueMutex);
				scheduler->workerCondition.wait(lock, [this]()
				{
					return !scheduler->running || !scheduler->pendingTaskQueue.empty();
				});

				if (!scheduler->running)// && scheduler->taskQueue.empty())
					return;

				task = std::move(scheduler->pendingTaskQueue.top());
				scheduler->pendingTaskQueue.pop();

				scheduler->workerToTaskMap[workerIndex] = task.taskId;
			}

			mutex->lock();

			bool canReschedule = task.run();
			if (canReschedule)
			{
				scheduler->reschedule(std::move(task));
			}

			{
				std::unique_lock<std::mutex> lock(scheduler->queueMutex);
				scheduler->workerToTaskMap[workerIndex] = InvalidTaskId;

				if (!canReschedule)
				{
					TaskFutures::iterator it = scheduler->taskFutures.find(task.taskId);
					if (it != scheduler->taskFutures.end())
						scheduler->taskFutures.erase(it);
				}
			}

			mutex->unlock();


		}

		TS_LOG_DEBUG("Thread Worker %u quitting.", workerIndex);
	}
};

SchedulerTaskId ThreadScheduler::ScheduledTask::nextTaskId = 1;

ThreadScheduler::ThreadScheduler()
{
	gigaton.registerClass(this);
}

ThreadScheduler::~ThreadScheduler()
{
	gigaton.unregisterClass(this);
}

bool ThreadScheduler::initialize()
{
// 	const SizeType numWorkers = math::min(TS_MAX_THREAD_POOL_THREAD_COUNT, ThreadScheduler::numHardwareThreads());
	const SizeType numWorkers = TS_MAX_THREAD_POOL_THREAD_COUNT;
	createBackgroundWorkers(numWorkers);

	return true;
}

void ThreadScheduler::deinitialize()
{
	destroyBackgroundWorkers();
}

void ThreadScheduler::createBackgroundWorkers(SizeType numWorkers)
{
	std::unique_lock<std::mutex> lock(queueMutex);

	TS_ASSERT(running == false && "Scheduler background workers have already been created.");
	running = true;

	backgroundScheduler.reset(new BackgroundScheduler(this));

	backgroundWorkers.reserve(numWorkers);
	workerMutexes.reserve(numWorkers);
	workerToTaskMap.resize(numWorkers, InvalidTaskId);
	for (SizeType index = 0; index < numWorkers; ++index)
	{
		workerMutexes.push_back(makeShared<std::mutex>());

		UniquePointer<BackgroundWorker> worker = makeUnique<BackgroundWorker>(this, index, workerMutexes.back());
		backgroundWorkers.push_back(std::move(worker));
	}
}

void ThreadScheduler::destroyBackgroundWorkers()
{
	{
		std::unique_lock<std::mutex> lock(queueMutex);
		running = false;
	}

	schedulerCondition.notify_all();
	workerCondition.notify_all();

	backgroundScheduler.reset();

	for (SizeType index = 0; index < backgroundWorkers.size(); ++index)
	{
		backgroundWorkers[index].reset();
	}
}

SizeType ThreadScheduler::getNumTasks() const
{
	std::unique_lock<std::mutex> lock(queueMutex);
	return (SizeType)(waitingTaskQueue.size() + pendingTaskQueue.size());
}

SizeType ThreadScheduler::getNumTasksInProgress() const
{
	std::unique_lock<std::mutex> lock(queueMutex);
	SizeType numTasks = 0;
	for (SchedulerTaskId id : workerToTaskMap)
	{
		if (id != InvalidTaskId)
			numTasks++;
	}
	return numTasks;
}

SizeType ThreadScheduler::getNumWorkers() const
{
	std::unique_lock<std::mutex> lock(queueMutex);
	return (SizeType)backgroundWorkers.size();
}

ThreadScheduler::SchedulerStats ThreadScheduler::getStats() const
{
// 	std::unique_lock<std::mutex> lock(queueMutex);
	SchedulerStats stats;

	stats.numBackgroundWorkers = (SizeType)backgroundWorkers.size();
	
	stats.numQueuedTasks = (SizeType)(waitingTaskQueue.size() + pendingTaskQueue.size());

	for (SchedulerTaskId id : workerToTaskMap)
		stats.numWorkedTasks += (id != InvalidTaskId ? 1 : 0);

	for (auto &it : waitingTaskQueue)
		stats.numIntervalTasks += (it.interval > TimeSpan::zero ? 1 : 0);
	for (auto &it : pendingTaskQueue)
		stats.numIntervalTasks += (it.interval > TimeSpan::zero ? 1 : 0);

	return stats;
}

bool ThreadScheduler::hasTasks() const
{
	std::unique_lock<std::mutex> lock(queueMutex);
	return !waitingTaskQueue.empty() && !pendingTaskQueue.empty();
}

void ThreadScheduler::clearTasks()
{
	{
		std::unique_lock<std::mutex> lock(queueMutex);
		waitingTaskQueue.clear();
		pendingTaskQueue.clear();
	}
	schedulerCondition.notify_all();
	workerCondition.notify_all();
}

bool ThreadScheduler::isTaskQueued(SchedulerTaskId taskId)
{
	std::unique_lock<std::mutex> lock(queueMutex);
	return isTaskQueuedUnsafe(taskId);
}

bool ThreadScheduler::isTaskQueuedUnsafe(SchedulerTaskId taskId)
{
	auto pred = [taskId](const ScheduledTask &t)
	{
		return t.taskId == taskId;
	};

	return (waitingTaskQueue.find_if(pred) != waitingTaskQueue.end()) ||
		   (pendingTaskQueue.find_if(pred) != pendingTaskQueue.end());
}

bool ThreadScheduler::cancelTask(SchedulerTaskId taskId)
{
	std::unique_lock<std::mutex> lock(queueMutex);

	auto pred = [taskId](const ScheduledTask &t)
	{
		return t.taskId == taskId;
	};

	return waitingTaskQueue.erase_if(pred) || pendingTaskQueue.erase_if(pred);
}

void ThreadScheduler::waitUntilTaskComplete(SchedulerTaskId taskId)
{
	TaskFuture *future = nullptr;

	{
		std::unique_lock<std::mutex> lock(queueMutex);

		TaskFutures::iterator futureIt = taskFutures.find(taskId);
		if (futureIt == taskFutures.end())
		{
			TS_PRINTF("Task %u is already complete.\n", taskId);
			return;
		}

		future = &futureIt->second;

		if (!isTaskQueuedUnsafe(taskId))
		{
			bool found = false;
			for (SchedulerTaskId id: workerToTaskMap)
			{
				if (id == taskId)
				{
					TS_PRINTF("waitUntilTaskComplete : Task ID %u is worked by worker %u\n", taskId, id);
					found = true;
					break;
				}
			}
			TS_ASSERT(found && "Task is not queued or being worked on.");
		}
		else
		{
			TS_PRINTF("waitUntilTaskComplete : Task ID %u is queued.\n", taskId);
		}
	}

	if (future == nullptr)
	{
		TS_PRINTF("Future is null! Task ID %u\n", taskId);
		return;
	}

	TS_PRINTF("Gonna wait for Task ID %u\n", taskId);

	try
	{
		if (!future->valid())
		{
			TS_PRINTF("Future is not even valid! Task ID %u\n", taskId);
			return;
		}

		while (true)
		{
			std::future_status status = future->wait_for(std::chrono::milliseconds(100));
			switch (status)
			{
				case std::future_status::timeout:  TS_PRINTF("Wait timeouted! Task ID %u\n", taskId); break;
				case std::future_status::ready:    TS_PRINTF("Wait is ready! Task ID %u\n", taskId); break;
				case std::future_status::deferred: TS_PRINTF("Wait is deferred! Task ID %u\n", taskId); break;
			}

			if (status == std::future_status::ready) break;
		}
	}
	catch (const std::future_error &e)
	{
		TS_PRINTF("\nTask ID %u wait exception: %s\n\n", taskId, e.what());
	}
}

SchedulerTaskId ThreadScheduler::scheduleThreadEntry(BaseThreadEntry *entry, TaskPriority priority, TimeSpan time_from_now)
{
	TS_ASSERT(entry != nullptr);

	ScheduledTaskFuture<void> future = scheduleOnce(priority, time_from_now, [=]()
	{
		TS_LOG_DEBUG("Entered scheduleThreadEntry task (%s)", entry->getDebugString());
		entry->entry();
		TS_LOG_DEBUG("Exiting scheduleThreadEntry task (%s)", entry->getDebugString());
	});

#if TS_BUILD != TS_FINALRELEASE
	{
		std::unique_lock<std::mutex> lock(queueMutex);
		Time now = Time::now();
		TS_PRINTF("waitingTaskQueue (%llu tasks):\n", waitingTaskQueue.size());
		for (auto &it : waitingTaskQueue)
		{
			TS_PRINTF("  ID %u : priority %d / time %lld ms\n",
				it.taskId,
				it.priority,
				(it.scheduledTime - now).getMilliseconds()
			);
		}
		TS_PRINTF("\n");
	}
#endif

	return future.getTaskId();
}

SizeType ThreadScheduler::numHardwareThreads()
{
	return (SizeType)std::thread::hardware_concurrency();
}

void ThreadScheduler::reschedule(ScheduledTask &&task)
{
	std::unique_lock<std::mutex> lock(queueMutex);

	task.reschedule();
	taskFutures[task.taskId] = std::move(task.getFuture());

	waitingTaskQueue.push(std::move(task));
}

TS_END_PACKAGE1()
