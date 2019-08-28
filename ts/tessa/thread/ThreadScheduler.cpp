#include "Precompiled.h"
#include "ThreadScheduler.h"

#include "ts/tessa/thread/AbstractThreadEntry.h"
#include "ts/tessa/thread/Thread.h"
#include "ts/tessa/thread/ThreadUtils.h"

#include "ts/tessa/Config.h"

#include <type_traits>

#pragma optimize("", off)

TS_DEFINE_MANAGER_TYPE(thread::ThreadScheduler);

TS_PACKAGE1(thread)

class ThreadScheduler::BackgroundScheduler : public AbstractThreadEntry
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
			MutexGuard lock(scheduler->queueMutex, MUTEXGUARD_DEBUGINFO());
			scheduler->schedulerCondition.waitFor(lock, TimeSpan::fromMilliseconds(10),
			[this]()
			{
				return !scheduler->running || !scheduler->waitingTaskQueue.empty();
			});

			if (!scheduler->running)// && scheduler->pendingTaskQueue.empty())
				return;

			if (scheduler->waitingTaskQueue.empty())
				continue;

			if (Time::now() >= scheduler->waitingTaskQueue.top()->scheduledTime)
			{
				// Move task from pending list to the active list
				scheduler->pendingTaskQueue.push(scheduler->waitingTaskQueue.top());
				scheduler->waitingTaskQueue.pop();

				lock.unlock();

				// Notifies any waiting worker to start handling it
				scheduler->workerCondition.notifyOne();
			}
		}
	}
};

class ThreadScheduler::BackgroundWorker : public AbstractThreadEntry
{
	ThreadScheduler *scheduler = nullptr;
	SizeType workerIndex = 0;

	Thread *thread;

public:
	BackgroundWorker(ThreadScheduler *scheduler, SizeType workerIndex)
		: scheduler(scheduler)
		, workerIndex(workerIndex)
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
			SharedScheduledTask task = nullptr;

			{
				MutexGuard lock(scheduler->queueMutex, MUTEXGUARD_DEBUGINFO());
				scheduler->workerCondition.wait(lock, [this]()
				{
					return !scheduler->running || !scheduler->pendingTaskQueue.empty();
				});

				if (!scheduler->running)// && scheduler->taskQueue.empty())
					return;

				task = scheduler->pendingTaskQueue.top();
				scheduler->pendingTaskQueue.pop();

				scheduler->workerToTaskMap[workerIndex] = task->taskId;
			}
			TS_ASSERT(task != nullptr);

			task->workedByWorkerIndex = workerIndex;
			bool canReschedule = task->run();
			task->workedByWorkerIndex = ScheduledTask::InvalidWorkerIndex;

			{
				MutexGuard lock(scheduler->queueMutex, MUTEXGUARD_DEBUGINFO());

				scheduler->workerToTaskMap[workerIndex] = InvalidTaskId;

				if (canReschedule)
				{
					task->reschedule();
					scheduler->waitingTaskQueue.push(task);
				}
				else
				{
					scheduler->incompleteTasks.erase(task->taskId);
				}
			}
		}

		TS_LOG_DEBUG("Thread Worker %u quitting.", workerIndex);
	}
};

SchedulerTaskId ScheduledTask::nextTaskId = 1;

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
	const SizeType numWorkers = TS_MAX_THREAD_POOL_THREAD_COUNT + 15;
	createBackgroundWorkers(numWorkers);

	return true;
}

void ThreadScheduler::deinitialize()
{
	destroyBackgroundWorkers();
}

void ThreadScheduler::createBackgroundWorkers(SizeType numWorkers)
{
	MutexGuard lock(queueMutex, MUTEXGUARD_DEBUGINFO());

	TS_ASSERT(running == false && "Scheduler background workers have already been created.");
	running = true;

	backgroundScheduler.reset(new BackgroundScheduler(this));

	backgroundWorkers.reserve(numWorkers);
	workerToTaskMap.resize(numWorkers, InvalidTaskId);

	for (SizeType index = 0; index < numWorkers; ++index)
	{
		UniquePointer<BackgroundWorker> worker = makeUnique<BackgroundWorker>(this, index);
		backgroundWorkers.push_back(std::move(worker));
	}
}

void ThreadScheduler::destroyBackgroundWorkers()
{
	{
		MutexGuard lock(queueMutex, MUTEXGUARD_DEBUGINFO());
		running = false;
	}

	schedulerCondition.notifyAll();
	workerCondition.notifyAll();

	backgroundScheduler.reset();

	for (SizeType index = 0; index < backgroundWorkers.size(); ++index)
	{
		backgroundWorkers[index].reset();
	}
}

SizeType ThreadScheduler::getNumTasks() const
{
	MutexGuard lock(queueMutex, MUTEXGUARD_DEBUGINFO());
	return (SizeType)(waitingTaskQueue.size() + pendingTaskQueue.size());
}

SizeType ThreadScheduler::getNumTasksInProgress() const
{
	MutexGuard lock(queueMutex, MUTEXGUARD_DEBUGINFO());
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
	MutexGuard lock(queueMutex, MUTEXGUARD_DEBUGINFO());
	return (SizeType)backgroundWorkers.size();
}

ThreadScheduler::SchedulerStats ThreadScheduler::getStats() const
{
// 	MutexGuard lock(queueMutex, MUTEXGUARD_DEBUGINFO());
	SchedulerStats stats;

	stats.numBackgroundWorkers = (SizeType)backgroundWorkers.size();
	
	stats.numQueuedTasks = (SizeType)(waitingTaskQueue.size() + pendingTaskQueue.size());

	for (SchedulerTaskId id : workerToTaskMap)
		stats.numWorkedTasks += (id != InvalidTaskId ? 1 : 0);

	for (auto &it : waitingTaskQueue)
		stats.numIntervalTasks += (it->interval > TimeSpan::zero ? 1 : 0);
	for (auto &it : pendingTaskQueue)
		stats.numIntervalTasks += (it->interval > TimeSpan::zero ? 1 : 0);

	return stats;
}

bool ThreadScheduler::hasTasks() const
{
	MutexGuard lock(queueMutex, MUTEXGUARD_DEBUGINFO());
	return !waitingTaskQueue.empty() && !pendingTaskQueue.empty();
}

// void ThreadScheduler::clearTasks()
// {
// 	{
// 		MutexGuard lock(queueMutex, MUTEXGUARD_DEBUGINFO());
// 		waitingTaskQueue.clear();
// 		pendingTaskQueue.clear();
// 		incompleteTasks.clear();
// 	}
// 	schedulerCondition.notifyAll();
// 	workerCondition.notifyAll();
// }

bool ThreadScheduler::isTaskQueued(SchedulerTaskId taskId)
{
	MutexGuard lock(queueMutex, MUTEXGUARD_DEBUGINFO());
	return isTaskQueuedUnsafe(taskId);
}

bool ThreadScheduler::isTaskQueuedUnsafe(SchedulerTaskId taskId)
{
	auto pred = [taskId](const SharedScheduledTask &t)
	{
		return t->taskId == taskId;
	};

	return (waitingTaskQueue.find_if(pred) != waitingTaskQueue.end()) ||
		   (pendingTaskQueue.find_if(pred) != pendingTaskQueue.end());
}

bool ThreadScheduler::cancelTask(SchedulerTaskId taskId)
{
	MutexGuard lock(queueMutex, MUTEXGUARD_DEBUGINFO());

	TasksList::iterator taskIt = incompleteTasks.find(taskId);
	if (taskIt == incompleteTasks.end())
		return false;

	auto pred = [taskId](const SharedScheduledTask &t)
	{
		return t->taskId == taskId;
	};

	bool erasedFromQueue = waitingTaskQueue.erase_if(pred) || pendingTaskQueue.erase_if(pred);
	if (erasedFromQueue)
		incompleteTasks.erase(taskIt);

	return erasedFromQueue;
}

void ThreadScheduler::waitUntilTaskComplete(SchedulerTaskId taskId)
{
	SharedScheduledTask task = nullptr;

	{
		MutexGuard lock(queueMutex, MUTEXGUARD_DEBUGINFO());

		TasksList::iterator taskIt = incompleteTasks.find(taskId);
		if (taskIt == incompleteTasks.end())
		{
// 			TS_PRINTF("Task ID %u was not found on incomplete tasks...\n", taskId);
			return;
		}

		task = taskIt->second;
	}

	if (task != nullptr)
	{
		task->waitForCompletion();
// 		TS_PRINTF("Task ID %u complete...\n", taskId);
	}
	else
	{
// 		TS_PRINTF("Task ID %u was found but is somehow null...\n", taskId);
	}
}

SchedulerTaskId ThreadScheduler::scheduleThreadEntry(AbstractThreadEntry *entry, TaskPriority priority, TimeSpan time_from_now)
{
	TS_ASSERT(entry != nullptr);

	ScheduledTaskFuture<void> future = scheduleOnce(priority, time_from_now, [=]()
	{
// 		TS_LOG_DEBUG("Entered scheduleThreadEntry task (%s)", entry->getDebugString());
		entry->entry();
// 		TS_LOG_DEBUG("Exiting scheduleThreadEntry task (%s)", entry->getDebugString());
	});

// #if TS_BUILD != TS_FINALRELEASE
// 	{
// 		MutexGuard lock(queueMutex, MUTEXGUARD_DEBUGINFO());
// 		Time now = Time::now();
// 		TS_PRINTF("waitingTaskQueue (%llu tasks):\n", waitingTaskQueue.size());
// 		for (auto &it : waitingTaskQueue)
// 		{
// 			TS_PRINTF("  ID %u : priority %d / time %lld ms\n",
// 				it->taskId,
// 				it->priority,
// 				(it->scheduledTime - now).getMilliseconds()
// 			);
// 		}
// 		TS_PRINTF("\n");
// 	}
// #endif

	return future.getTaskId();
}

SizeType ThreadScheduler::numHardwareThreads()
{
	return (SizeType)std::thread::hardware_concurrency();
}


TS_END_PACKAGE1()