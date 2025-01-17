#include "Precompiled.h"
#include "ThreadScheduler.h"

#include "ts/thread/AbstractThreadEntry.h"
#include "ts/thread/Thread.h"
#include "ts/thread/ThreadUtils.h"

#include <type_traits>

TS_DEFINE_MANAGER_TYPE(thread::ThreadScheduler);

TS_PACKAGE1(thread)

thread_local SchedulerTaskId ThreadScheduler::currentThreadTaskId = InvalidTaskId;

class ThreadScheduler::BackgroundScheduler : public AbstractThreadEntry
{
	ThreadScheduler *scheduler = nullptr;

	Thread *thread;

public:
	BackgroundScheduler(ThreadScheduler *scheduler)
		: scheduler(scheduler)
	{
		thread = Thread::createThread(this, "ThrdScheduler");
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
			MutexGuard lock(scheduler->queueMutex);
			scheduler->schedulerCondition.waitFor(lock, 10_ms,
			[this]()
			{
				return !scheduler->running || !scheduler->waitingTaskQueue.empty();
			});

			if (!scheduler->running)// && scheduler->waitingTaskQueue.empty())
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
			else
			{
				lock.unlock();
			}

			Thread::sleep(10_ms);
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
		thread = Thread::createThread(this, TS_FMT("Worker %u", workerIndex));
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
				MutexGuard lock(scheduler->queueMutex);
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

			ThreadScheduler::currentThreadTaskId = task->taskId;

			task->workedByWorkerIndex = workerIndex;
			bool canReschedule = task->run();
			task->workedByWorkerIndex = ScheduledTask::InvalidWorkerIndex;

			ThreadScheduler::currentThreadTaskId = InvalidTaskId;

			{
				MutexGuard lock(scheduler->queueMutex);

				scheduler->workerToTaskMap[workerIndex] = InvalidTaskId;

				if (canReschedule && scheduler->cancelledTasks.count(task->taskId) == 0)
				{
					task->reschedule();
					scheduler->waitingTaskQueue.push(task);
					scheduler->schedulerCondition.notifyAll();
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
	MutexGuard lock(queueMutex);

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
		MutexGuard lock(queueMutex);
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
	MutexGuard lock(queueMutex);
	return (SizeType)(waitingTaskQueue.size() + pendingTaskQueue.size());
}

SizeType ThreadScheduler::getNumTasksInProgress() const
{
	MutexGuard lock(queueMutex);
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
	MutexGuard lock(queueMutex);
	return (SizeType)backgroundWorkers.size();
}

ThreadScheduler::SchedulerStats ThreadScheduler::getStats() const
{
// 	MutexGuard lock(queueMutex);
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
	MutexGuard lock(queueMutex);
	return !waitingTaskQueue.empty() && !pendingTaskQueue.empty();
}

// void ThreadScheduler::clearTasks()
// {
// 	{
// 		MutexGuard lock(queueMutex);
// 		waitingTaskQueue.clear();
// 		pendingTaskQueue.clear();
// 		incompleteTasks.clear();
// 	}
// 	schedulerCondition.notifyAll();
// 	workerCondition.notifyAll();
// }

bool ThreadScheduler::isTaskQueued(SchedulerTaskId taskId)
{
	MutexGuard lock(queueMutex);
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

bool ThreadScheduler::cancelTask(SchedulerTaskId taskId, bool waitCompletion)
{
	MutexGuard lock(queueMutex);

	TasksList::iterator taskIt = incompleteTasks.find(taskId);
	if (taskIt == incompleteTasks.end())
		return false;

	cancelledTasks.insert(taskId);

	auto tryEraseFromQueue = [this, taskId]() -> bool
	{
		const auto pred = [taskId](const SharedScheduledTask &t)
		{
			return t->taskId == taskId;
		};

		TaskPriorityQueue::const_iterator queueIter = waitingTaskQueue.find_if(pred);
		if (queueIter != waitingTaskQueue.end())
		{
			waitingTaskQueue.erase(queueIter);
			return true;
		}

		queueIter = pendingTaskQueue.find_if(pred);
		if (queueIter != pendingTaskQueue.end())
		{
			pendingTaskQueue.erase(queueIter);
			return true;
		}

		return false;
	};

	bool wasErased = tryEraseFromQueue();
	if (wasErased)
	{
		incompleteTasks.erase(taskIt);
	}
	else if (waitCompletion)
	{
		lock.unlock();
		taskIt->second->waitForCompletion();
		lock.lock();
	}

	return true;
}

bool ThreadScheduler::isTaskCancelled(SchedulerTaskId taskId) const
{
	return cancelledTasks.count(taskId) > 0;
}

void ThreadScheduler::waitUntilTaskComplete(SchedulerTaskId taskId)
{
	SharedScheduledTask task = nullptr;

	{
		MutexGuard lock(queueMutex);

		TasksList::iterator taskIt = incompleteTasks.find(taskId);
		if (taskIt == incompleteTasks.end())
			return;

		task = taskIt->second;
	}

	if (task != nullptr)
		task->waitForCompletion();
}

SchedulerTaskId ThreadScheduler::scheduleThreadEntry(AbstractThreadEntry *entry, TaskPriority priority, TimeSpan time_from_now)
{
	TS_ASSERT(entry != nullptr);

	ScheduledTaskFuture<void> future = scheduleOnce(priority, time_from_now, [=]()
	{
		entry->entry();
	});

	return future.getTaskId();
}

SizeType ThreadScheduler::numHardwareThreads()
{
	return (SizeType)std::thread::hardware_concurrency();
}

SchedulerTaskId ThreadScheduler::getCurrentThreadTaskId()
{
	return currentThreadTaskId;
}


TS_END_PACKAGE1()

