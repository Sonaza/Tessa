#pragma once

#include "ts/tessa/system/AbstractManagerBase.h"

#include "ts/tessa/threading/BaseThreadEntry.h"
#include "ts/tessa/threading/TaskCompletionFuture.h"

#include "ts/tessa/util/PriorityQueue.h"

#include "ts/tessa/time/Time.h"
#include "ts/tessa/time/TimeSpan.h"

#include <thread>
#include <condition_variable>
#include <functional>
#include <memory>
#include <chrono>
#include <future>

TS_PACKAGE1(threading)

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
		return future.wait_for(std::chrono::milliseconds(waitTime.getMilliseconds()));
	}

	SchedulerTaskId getTaskId() const
	{
		return taskId;
	}

private:
	std::shared_future<ReturnType> future;
	SchedulerTaskId taskId = InvalidTaskId;
};

class ThreadScheduler : public system::AbstractManagerBase
{
	TS_DECLARE_MANAGER_TYPE(threading::ThreadScheduler);

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
	bool cancelTask(SchedulerTaskId taskId);

	// Blocks until given task id is complete (returns immediately if task wasn't found)
	// If used with interval tasks it will wait until the next run is complete.
	void waitUntilTaskComplete(SchedulerTaskId taskId);

	// Clears all tasks from queues.
	void clearTasks();

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
		BaseThreadEntry *entry,
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
		TimeSpan interval,
		Function &&f, Args&&... args);

	// For passing in instanced class methods
	template<class Class, class... Args>
	SchedulerTaskId scheduleWithInterval(
		TaskPriority priority,
		TimeSpan interval,
		bool(Class::*taskFunction)(Args...), Class *instance, Args&&... args);
	
	static SizeType numHardwareThreads();

private:
	void createBackgroundWorkers(SizeType numWorkers);
	void destroyBackgroundWorkers();

	bool isTaskQueuedUnsafe(SchedulerTaskId taskId);

	struct ScheduledTask
	{
		friend class ThreadScheduler;

		ScheduledTask()
		{
// 			TS_ASSERT(!"Hello");
		}

		ScheduledTask(TaskPriority priority, Time time, std::function<bool()> &&task)
			: initialized(true)
			, scheduledTime(time)
			, interval(TimeSpan::zero)
			, task(std::move(task))
			, taskId(ScheduledTask::nextTaskId++)
			, priority(priority)
		{
			promise = makeUnique<TaskCompletionPromise>();
		}

		ScheduledTask(TaskPriority priority, TimeSpan interval, std::function<bool()> &&task)
			: initialized(true)
			, scheduledTime(Time::now())
			, interval(interval)
			, task(std::move(task))
			, taskId(ScheduledTask::nextTaskId++)
			, priority(priority)
		{
			promise = makeUnique<TaskCompletionPromise>();
		}

		ScheduledTask(const ScheduledTask &other) = delete;
// 		{
// 			TS_ASSERT(!"Copies aren't very nice.");
// 		}

		ScheduledTask &operator=(const ScheduledTask &other) = delete;
// 		{
// 			TS_ASSERT(!"sadfkasogakdofgkdfgo");
// 			return *this;
// 		}

		ScheduledTask(ScheduledTask &&other) noexcept
		{
			*this = std::move(other);
		}

		ScheduledTask &operator=(ScheduledTask &&other) noexcept
		{
			if (this != &other)
			{
				std::swap(initialized, other.initialized);
				std::swap(scheduledTime, other.scheduledTime);
				std::swap(interval, other.interval);
				task = std::move(other.task);
				std::swap(taskId, other.taskId);
				promise = std::move(other.promise);
				std::swap(priority, other.priority);
			}
			return *this;
		}

		bool isValid() const
		{
			return initialized == true && task && promise != nullptr;
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

			bool reschedulable = std::invoke(task) && (interval > TimeSpan::zero);

			TS_ASSERT(promise != nullptr && "My promises are empty? What?");
			promise->signalCompletion();

			return reschedulable;
		}

		bool initialized = false;
		SchedulerTaskId taskId = InvalidTaskId;
		Time scheduledTime;
		TimeSpan interval;
		std::function<bool()> task;
		UniquePointer<TaskCompletionPromise> promise;
		TaskPriority priority = Priority_Normal;

		static SchedulerTaskId nextTaskId;
	};

	typedef util::PriorityQueue<ScheduledTask> TaskQueueType;
	TaskQueueType waitingTaskQueue;
	TaskQueueType pendingTaskQueue;

	typedef std::map<SchedulerTaskId, TaskCompletionFuture> TaskCompletionFutures;
	TaskCompletionFutures taskFutures;

	// Matches worker ids to actively worked tasks
	std::map<SizeType, SchedulerTaskId> workerToTaskMap;

	template <class ReturnType>
	ScheduledTaskFuture<ReturnType> scheduleOnceImpl(
		TaskPriority priority, TimeSpan time_from_now, std::function<ReturnType()> &&f);

	SchedulerTaskId scheduleWithIntervalImpl(
		TaskPriority priority, TimeSpan interval, std::function<bool()> &&f);

	void reschedule(ScheduledTask &&task);

	class BackgroundScheduler;
	friend class BackgroundScheduler;
	ScopedPointer<BackgroundScheduler> backgroundScheduler;

	class BackgroundWorker;
	friend class BackgroundWorker;
	std::vector<UniquePointer<BackgroundWorker>> backgroundWorkers;

	bool running = false;

	std::condition_variable schedulerCondition;
	std::condition_variable workerCondition;
	std::condition_variable taskStartedCondition;
	mutable std::mutex queueMutex;
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
	{
		std::unique_lock<std::mutex> lock(queueMutex);
		if (running)
		{
			ScheduledTask task(priority, Time::now() + time_from_now, [packagedTask]()
			{
				std::invoke(*packagedTask);
// 				(*packagedTask)();
				return false; // Schedule once task is not reschedulable
			});

			future.taskId = task.taskId;
			taskFutures.insert(std::make_pair(task.taskId, task.promise->getFuture()));

			waitingTaskQueue.push(std::move(task));
		}
	}

	schedulerCondition.notify_all();

	return future;
}

template<class Function, class... Args>
SchedulerTaskId ThreadScheduler::scheduleWithInterval(
	TaskPriority priority,
	TimeSpan interval,
	Function &&taskFunction, Args&&... args)
{
	typedef typename std::result_of<Function(Args...)>::type ReturnType;
	static_assert(std::is_same<bool, ReturnType>::value, "Interval schedule callback should return a boolean.");

	return scheduleWithIntervalImpl(
		priority,
		interval,
		std::move(std::bind(std::forward<Function>(taskFunction), std::forward<Args>(args)...))
	);
}

template<class Class, class... Args>
SchedulerTaskId ts::threading::ThreadScheduler::scheduleWithInterval(
	TaskPriority priority,
	TimeSpan interval,
	bool(Class::*taskFunction)(Args...), Class *instance, Args&&... args)
{
	std::function<bool()> bound = std::bind(taskFunction, instance, std::forward<Args>(args)...);
	return scheduleWithIntervalImpl(
		priority,
		interval,
		std::move(std::bind(taskFunction, instance, std::forward<Args>(args)...))
	);
}

inline SchedulerTaskId ThreadScheduler::scheduleWithIntervalImpl(
	TaskPriority priority,
	TimeSpan interval,
	std::function<bool()> &&function)
{
	SharedPointer<std::function<bool()>> sharedTask =
		makeShared<std::function<bool()>>(std::move(function));

	SchedulerTaskId createdTaskId = InvalidTaskId;
	{
		std::unique_lock<std::mutex> lock(queueMutex);
		if (running)
		{
			ScheduledTask task(priority, interval, [sharedTask]()
			{
				// Task return value determines if it will be rescheduled
				return std::invoke(*sharedTask);
			});

			createdTaskId = task.taskId;
			taskFutures.insert(std::make_pair(task.taskId, task.promise->getFuture()));

			waitingTaskQueue.push(std::move(task));
		}
	}

	schedulerCondition.notify_all();

	return createdTaskId;
}

TS_END_PACKAGE1()
