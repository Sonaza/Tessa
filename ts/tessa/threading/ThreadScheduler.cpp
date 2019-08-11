#include "Precompiled.h"
#include "ThreadScheduler.h"

#include "ts/tessa/threading/BaseThreadEntry.h"
#include "ts/tessa/threading/Thread.h"
#include "ts/tessa/threading/ThreadUtils.h"

#include "ts/tessa/Config.h"

TS_DEFINE_SYSTEM_MANAGER_TYPE(threading::ThreadScheduler);

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
				return !scheduler->running || !scheduler->pendingTaskQueue.empty();
			});

			if (!scheduler->running)// && scheduler->pendingTaskQueue.empty())
				return;

			if (scheduler->pendingTaskQueue.empty())
				continue;

			if (Time::now() >= scheduler->pendingTaskQueue.top().scheduledTime)
			{
				// Move task from pending list to the active list
				scheduler->taskQueue.push(std::move(scheduler->pendingTaskQueue.top()));
				scheduler->pendingTaskQueue.pop();

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
			ScheduledTask task;
			{
				std::unique_lock<std::mutex> lock(scheduler->queueMutex);
				scheduler->workerCondition.wait(lock, [this]()
				{
					return !scheduler->running || !scheduler->taskQueue.empty();
				});

				if (!scheduler->running)// && scheduler->taskQueue.empty())
					return;

				task = std::move(scheduler->taskQueue.top());
				scheduler->taskQueue.pop();
			}

			task();

			if (task.interval > TimeSpan::zero)
			{
				scheduler->reschedule(std::move(task));
			}
		}

		TS_LOG_DEBUG("Thread Worker %u quitting.", workerIndex);
	}
};

SchedulerTaskId ThreadScheduler::ScheduledTask::nextTaskId = 0;

ThreadScheduler::ThreadScheduler(system::BaseApplication *application)
	: SystemManagerBase(application)
{
	TS_GIGATON_REGISTER_CLASS(this);
}

ThreadScheduler::~ThreadScheduler()
{
	TS_GIGATON_UNREGISTER_CLASS(this);
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

void ThreadScheduler::update(const TimeSpan deltaTime)
{

}

void ThreadScheduler::createBackgroundWorkers(SizeType numWorkers)
{
	std::unique_lock<std::mutex> lock(queueMutex);

	TS_ASSERT(running == false && "Scheduler background workers have already been created.");
	running = true;

	backgroundScheduler.reset(new BackgroundScheduler(this));

	backgroundWorkers.reserve(numWorkers);
	for (SizeType index = 0; index < numWorkers; ++index)
	{
		UniquePointer<BackgroundWorker> worker = makeUnique<BackgroundWorker>(this, index);
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

	for (SizeType index = 0; index< backgroundWorkers.size(); ++index)
	{
		backgroundWorkers[index].reset();
	}
	backgroundScheduler.reset();
}

SizeType ThreadScheduler::numTasks() const
{
	std::unique_lock<std::mutex> lock(queueMutex);
	return (SizeType)(pendingTaskQueue.size() + taskQueue.size());
}

bool ThreadScheduler::hasTasks() const
{
	std::unique_lock<std::mutex> lock(queueMutex);
	return !pendingTaskQueue.empty() && !taskQueue.empty();
}

void ThreadScheduler::clearTasks()
{
	{
		std::unique_lock<std::mutex> lock(queueMutex);
		pendingTaskQueue.clear();
		taskQueue.clear();
	}
	schedulerCondition.notify_all();
	workerCondition.notify_all();
}

void ThreadScheduler::cancelIntervalTask(SchedulerTaskId taskId)
{
	std::unique_lock<std::mutex> lock(queueMutex);
	std::remove_if(pendingTaskQueue.begin(), pendingTaskQueue.end(), [taskId](const ScheduledTask &t)
	{
		return t.taskId == taskId;
	});
}

SizeType ThreadScheduler::numHardwareThreads()
{
	return (SizeType)std::thread::hardware_concurrency();
}

void ThreadScheduler::reschedule(ScheduledTask &&task)
{
	std::unique_lock<std::mutex> lock(queueMutex);

	task.scheduledTime = Time::now() + task.interval;
	pendingTaskQueue.push(std::move(task));
}

TS_END_PACKAGE1()
