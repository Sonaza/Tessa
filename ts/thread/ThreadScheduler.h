#pragma once

#include "ts/engine/system/AbstractManagerBase.h"

#include "ts/thread/AbstractThreadEntry.h"
#include "ts/thread/ConditionVariable.h"
#include "ts/thread/Mutex.h"
#include "ts/thread/MutexGuard.h"

#include "ts/container/PriorityQueue.h"

#include "ts/time/Time.h"
#include "ts/time/TimeSpan.h"

#include <functional>
#include <memory>
#include <chrono>
#include <set>
#include <future>

TS_PACKAGE1(thread)

typedef SizeType SchedulerTaskId;
static const SchedulerTaskId InvalidTaskId = ~0U;

enum TaskPriority
{
	Priority_Critical = 0,
	Priority_High     = 1,
	Priority_Normal   = 2,
	Priority_Low      = 3,
	Priority_VeryLow  = 4,
};

/*********************************************************
* Scheduled task container
*/

template<class ReturnType>
class ScheduledTaskFuture
{
	friend class ThreadScheduler;

public:
	ScheduledTaskFuture() = delete;

	ScheduledTaskFuture(const ScheduledTaskFuture &other)
	{
		*this = other;
	}

	ScheduledTaskFuture &operator=(const ScheduledTaskFuture &other)
	{
		if (this != &other)
		{
			future = other.future;
			taskId = other.taskId;
		}
		return *this;
	}

	ScheduledTaskFuture(ScheduledTaskFuture &&other)
	{
		*this = std::move(other);
	}

	ScheduledTaskFuture &operator=(ScheduledTaskFuture &&other)
	{
		if (this != &other)
		{
			future = std::move(other.future);
			taskId = other.taskId;
			other.taskId = InvalidTaskId;
		}
		return *this;
	}

	ScheduledTaskFuture(std::future<ReturnType> &&future)
		: future(std::move(future))
		, taskId(InvalidTaskId)
	{
	}

	ReturnType getResult() const
	{
		return future.get();
	}

	bool isValid() const
	{
		return future.valid();
	}

	void wait() const
	{
		future.wait();
	}

	std::future_status waitFor(TimeSpan waitTime) const
	{
		return future.wait_for(std::chrono::microseconds(waitTime.getMicroseconds()));
	}

	SchedulerTaskId getTaskId() const
	{
		return taskId;
	}

private:
	std::shared_future<ReturnType> future;
	SchedulerTaskId taskId = InvalidTaskId;
};

/*********************************************************
 * Scheduled task container
 */

struct ScheduledTask
{
	friend class ThreadScheduler;

	ScheduledTask() = delete;

	ScheduledTask(TaskPriority priority, Time time, std::function<bool()> &&task)
		: initialized(true)
		, scheduledTime(time)
		, interval(TimeSpan::zero)
		, task(std::move(task))
		, taskId(ScheduledTask::nextTaskId++)
		, priority(priority)
	{
		future = promise.get_future();
	}

	ScheduledTask(TaskPriority priority, TimeSpan interval, bool startImmediately, std::function<bool()> &&task)
		: initialized(true)
		, scheduledTime(startImmediately ? Time::now() : Time::now() + interval)
		, interval(interval)
		, task(std::move(task))
		, taskId(ScheduledTask::nextTaskId++)
		, priority(priority)
	{
		future = promise.get_future();
	}

	// Non-copyable
	ScheduledTask(const ScheduledTask &other) = delete;
	ScheduledTask &operator=(const ScheduledTask &other) = delete;
	// Non-movable
	ScheduledTask(ScheduledTask &&other) noexcept = delete;
	ScheduledTask &operator=(ScheduledTask &&other) noexcept = delete;

	bool isValid() const
	{
		return initialized == true && task;
	}

	bool operator<(const ScheduledTask &rhs) const
	{
		return priority < rhs.priority ||
			(priority == rhs.priority && scheduledTime < rhs.scheduledTime) ||
			(priority == rhs.priority && scheduledTime == rhs.scheduledTime && taskId < rhs.taskId);
	}

private:
	// Returns if task should be rescheduled
	bool run()
	{
		TS_ASSERT(isValid() && "Trying to run an invalid/uninitialized task.");
		TS_ASSERT(completed == false && "Trying to run already completed.");

		bool reschedulable = std::invoke(task) && (interval > TimeSpan::zero);
		promise.set_value();
		completed = true;

		return reschedulable;
	}

	// Resets parameters for interval task rescheduling
	void reschedule()
	{
		TS_ASSERT(interval > TimeSpan::zero && "Task with zero interval is not valid for rescheduling.");

		scheduledTime = Time::now() + interval;

		promise = std::promise<void>();
		future = promise.get_future();

		completed = false;
	}

	void waitForCompletion()
	{
		if (!completed)
		{
			TS_PRINTF("waitForCompletion() Task ID %u : waiting\n", taskId);
			future.wait();
		}
		else
		{
			TS_PRINTF("waitForCompletion() Task UD %u : was already completed\n", taskId);
		}
	}

	bool initialized = false;
	bool completed = false;

	SchedulerTaskId taskId = InvalidTaskId;

	TaskPriority priority = Priority_Normal;
	Time scheduledTime;
	TimeSpan interval;

	std::function<bool()> task;

	std::promise<void> promise;
	std::future<void> future;

	static const SizeType InvalidWorkerIndex = ~0U;
	std::atomic<SizeType> workedByWorkerIndex = InvalidWorkerIndex;

	static SchedulerTaskId nextTaskId;
};

struct ScheduledTaskSharedPointerSorter
{
	bool operator()(const SharedPointer<ScheduledTask> &lhs, const SharedPointer<ScheduledTask> &rhs)
	{
		return *lhs < *rhs;
	}
};

class ThreadScheduler : public engine::system::AbstractManagerBase
{
	TS_DECLARE_MANAGER_TYPE(thread::ThreadScheduler);

public:
	ThreadScheduler();
	virtual ~ThreadScheduler();

	virtual bool initialize();
	virtual void deinitialize();

	bool hasTasks() const;
	SizeType getNumTasks() const;
	SizeType getNumTasksInProgress() const;
	SizeType getNumWorkers() const;

	struct SchedulerStats
	{
		SizeType numBackgroundWorkers = 0;
		SizeType numQueuedTasks = 0;
		SizeType numWorkedTasks = 0;
		SizeType numIntervalTasks = 0;
	};
	SchedulerStats getStats() const;

	// Returns true if the task is currently in the queue (waiting or pending)
	bool isTaskQueued(SchedulerTaskId taskId);

	// Returns true if task was cancelled succesfully, false if task wasn't found
	bool cancelTask(SchedulerTaskId taskId, bool waitCompletion);
	bool isTaskCancelled(SchedulerTaskId taskId) const;

	// Blocks until given task id is complete (returns immediately if task wasn't found)
	// Can't be used with interval tasks.
	void waitUntilTaskComplete(SchedulerTaskId taskId);

	// Clears all tasks from queues.
// 	void clearTasks();

	/*  Schedule thread entry:  Instead of starting a new thread of its own, a thread entry can execute
	 *                          its code with the scheduler's workers. Do note, if queuing up several
	 *                          tasks that take a long time, it may cause other tasks to be blocked
	 *                          due to the pool of workers all being busy.
	 * 
	 *  Schedule once:          A task is executed only once, after the given timeout has expired.
	 *                          The task function is allowed to return a value via the task future.
	 *
	 *  Schedule with interval: A task is executed with a set interval, until cancelled by cancelTask
	 *                          or the task method returns false on completion.
	 *                          The task function cannot return a value via a task future.
	 *
	 *  Priority determines the order of task execution with the highest priority tasks being
	 *  the first to be completed. Useful if there are several low priority tasks that can
	 *  be completed later and giving way for the occasional high priority task.
	 */

	SchedulerTaskId scheduleThreadEntry(
		AbstractThreadEntry *entry,
		TaskPriority priority = Priority_Normal,
		TimeSpan time_from_now = TimeSpan::zero);

	// For passing in function pointers and lambda functions
	template<class Function, class... Args>
	ScheduledTaskFuture<typename std::result_of<Function(Args...)>::type> scheduleOnce(
		TaskPriority priority,
		TimeSpan time_from_now,
		Function &&f, Args&&... args);

	// For passing in instanced class methods
	template<class ReturnType, class Class, class... Args>
	ScheduledTaskFuture<ReturnType> scheduleOnce(
		TaskPriority priority,
		TimeSpan time_from_now,
		ReturnType(Class::*taskFunction)(Args...), Class *instance, Args&&... args);

	// For passing in function pointers and lambda functions
	template<class Function, class... Args>
	SchedulerTaskId scheduleWithInterval(
		TaskPriority priority,
		TimeSpan interval, bool startImmediately,
		Function &&f, Args&&... args);

	// For passing in instanced class methods
	template<class Class, class... Args>
	SchedulerTaskId scheduleWithInterval(
		TaskPriority priority,
		TimeSpan interval, bool startImmediately,
		bool(Class::*taskFunction)(Args...), Class *instance, Args&&... args);
	
	static SizeType numHardwareThreads();

	// Returns current task id being executed in the current thread.
	// Returns InvalidTaskId if no task is currently being worked on.
	// Only valid for worker threads.
	static SchedulerTaskId getCurrentThreadTaskId();

private:
	static thread_local SchedulerTaskId currentThreadTaskId;

	void createBackgroundWorkers(SizeType numWorkers);
	void destroyBackgroundWorkers();

	bool isTaskQueuedUnsafe(SchedulerTaskId taskId);

	typedef SharedPointer<ScheduledTask> SharedScheduledTask;
	typedef util::PriorityQueue<SharedScheduledTask, ScheduledTaskSharedPointerSorter> TaskPriorityQueue;

	// Holder for all incomplete tasks
	typedef std::map<SchedulerTaskId, SharedScheduledTask> TasksList;
	TasksList incompleteTasks;

	// Queue for waiting tasks, i.e. ones where scheduled time has not yet passed.
	TaskPriorityQueue waitingTaskQueue;
	// Queue for pending tasks, i.e. tasks that are ready for execution
	// and a worker can start processing whenever able.
	TaskPriorityQueue pendingTaskQueue;

	// Keeps note of any task that was cancelled, important for interval tasks to prevent rescheduling.
	std::set<SchedulerTaskId> cancelledTasks;

	// Matches worker ids to actively worked tasks
	std::vector<SchedulerTaskId> workerToTaskMap;

	template <class ReturnType>
	ScheduledTaskFuture<ReturnType> scheduleOnceImpl(
		TaskPriority priority, TimeSpan time_from_now, std::function<ReturnType()> &&f);

	SchedulerTaskId scheduleWithIntervalImpl(
		TaskPriority priority, TimeSpan interval, bool startImmediately, std::function<bool()> &&f);

	class BackgroundScheduler;
	friend class BackgroundScheduler;
	ScopedPointer<BackgroundScheduler> backgroundScheduler;

	class BackgroundWorker;
	friend class BackgroundWorker;
	std::vector<UniquePointer<BackgroundWorker>> backgroundWorkers;

	std::atomic_bool running = false;

	ConditionVariable schedulerCondition;
	ConditionVariable workerCondition;
	mutable Mutex queueMutex;
};

template<class Function, class... Args>
ScheduledTaskFuture<typename std::result_of<Function(Args...)>::type> ThreadScheduler::scheduleOnce(
	TaskPriority priority,
	TimeSpan time_from_now,
	Function &&taskFunction, Args&&... args)
{
	typedef typename std::result_of<Function(Args...)>::type ReturnType;

	return scheduleOnceImpl<ReturnType>(
		priority,
		time_from_now,
		std::move(std::bind(std::forward<Function>(taskFunction), std::forward<Args>(args)...))
	);
}

template<class ReturnType, class Class, class... Args>
ScheduledTaskFuture<ReturnType> ThreadScheduler::scheduleOnce(
	TaskPriority priority,
	TimeSpan time_from_now,
	ReturnType(Class::*taskFunction)(Args...), Class *instance, Args&&... args)
{
	return scheduleOnceImpl<ReturnType>(
		priority,
		time_from_now,
		std::move(std::bind(taskFunction, instance, std::forward<Args>(args)...))
	);
}

template <class ReturnType>
ScheduledTaskFuture<ReturnType> ThreadScheduler::scheduleOnceImpl(
	TaskPriority priority,
	TimeSpan time_from_now,
	std::function<ReturnType()> &&function)
{
	SharedPointer<std::packaged_task<ReturnType()>> packagedTask =
		makeShared<std::packaged_task<ReturnType()>>(function);

	ScheduledTaskFuture<ReturnType> future = packagedTask->get_future();
	if (running)
	{
// 		ScheduledTask task(priority, Time::now() + time_from_now, [packagedTask]()
		SharedScheduledTask task = makeShared<ScheduledTask>(
			priority,
			Time::now() + time_from_now,
			[packagedTask]()
			{
				std::invoke(*packagedTask);
				return false; // Schedule once task is not reschedulable
			}
		);

		MutexGuard lock(queueMutex);
		future.taskId = task->taskId;
		incompleteTasks.insert(std::make_pair(task->taskId, task));
		waitingTaskQueue.push(task);
	}

	schedulerCondition.notifyAll();

	return future;
}

template<class Function, class... Args>
SchedulerTaskId ThreadScheduler::scheduleWithInterval(
	TaskPriority priority,
	TimeSpan interval, bool startImmediately,
	Function &&taskFunction, Args&&... args)
{
	typedef typename std::result_of<Function(Args...)>::type ReturnType;
	static_assert(std::is_same<bool, ReturnType>::value, "Interval schedule callback should return a boolean.");

	return scheduleWithIntervalImpl(
		priority,
		interval,
		startImmediately,
		std::move(std::bind(std::forward<Function>(taskFunction), std::forward<Args>(args)...))
	);
}

template<class Class, class... Args>
SchedulerTaskId ts::thread::ThreadScheduler::scheduleWithInterval(
	TaskPriority priority,
	TimeSpan interval, bool startImmediately,
	bool(Class::*taskFunction)(Args...), Class *instance, Args&&... args)
{
	std::function<bool()> bound = std::bind(taskFunction, instance, std::forward<Args>(args)...);
	return scheduleWithIntervalImpl(
		priority,
		interval,
		startImmediately,
		std::move(std::bind(taskFunction, instance, std::forward<Args>(args)...))
	);
}

inline SchedulerTaskId ThreadScheduler::scheduleWithIntervalImpl(
	TaskPriority priority,
	TimeSpan interval, bool startImmediately,
	std::function<bool()> &&function)
{
	SharedPointer<std::function<bool()>> sharedTask =
		makeShared<std::function<bool()>>(std::move(function));

	SchedulerTaskId createdTaskId = InvalidTaskId;
	if (running)
	{
// 		ScheduledTask task(priority, interval, [sharedTask]()
		SharedScheduledTask task = makeShared<ScheduledTask>(
			priority,
			interval,
			startImmediately,
			[sharedTask]()
			{
				// Task return value determines if it will be rescheduled
				return std::invoke(*sharedTask);
			}
		);
		createdTaskId = task->taskId;

		MutexGuard lock(queueMutex);
		incompleteTasks.insert(std::make_pair(task->taskId, task));
		waitingTaskQueue.push(task);
	}

	schedulerCondition.notifyAll();

	return createdTaskId;
}

TS_END_PACKAGE1()
