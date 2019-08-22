#pragma once

#include "ts/tessa/system/AbstractManagerBase.h"

#include "ts/tessa/threading/BaseThreadEntry.h"
#include "ts/tessa/threading/TaskCompletionFuture.h"

#include "ts/tessa/util/IterablePriorityQueue.h"

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

	SizeType numTasks() const;
	bool hasTasks() const;

	void clearTasks();

	bool isTaskQueued(SchedulerTaskId taskId);
	bool isTaskQueuedUnsafe(SchedulerTaskId taskId);

	// Returns true if task was cancelled succesfully, false if task wasn't found
	bool cancelTask(SchedulerTaskId taskId);
	void waitUntilTaskComplete(SchedulerTaskId taskId);

	SchedulerTaskId scheduleThreadEntry(BaseThreadEntry *entry);

	// For passing in function pointers and lambda functions
	template<class Function, class... Args>
	ScheduledTaskFuture<typename std::result_of<Function(Args...)>::type> scheduleOnce(TimeSpan time_from_now, Function &&f, Args&&... args);

	// For passing in instanced class methods
	template<class ReturnType, class Class, class... Args>
	ScheduledTaskFuture<ReturnType> scheduleOnce(TimeSpan time_from_now, ReturnType(Class::*taskFunction)(Args...), Class *instance, Args&&... args);

	// For passing in function pointers and lambda functions
	template<class Function, class... Args>
	SchedulerTaskId scheduleWithInterval(TimeSpan interval, Function &&f, Args&&... args);

	// For passing in instanced class methods
	template<class Class, class... Args>
	SchedulerTaskId scheduleWithInterval(TimeSpan interval, bool(Class::*taskFunction)(Args...), Class *instance, Args&&... args);
	
	static SizeType numHardwareThreads();

private:
	void createBackgroundWorkers(SizeType numWorkers);
	void destroyBackgroundWorkers();

	struct ScheduledTask
	{
		ScheduledTask()
		{
// 			TS_ASSERT(!"Hello");
		}

		ScheduledTask(Time time, std::function<bool()> &&task)
			: initialized(true)
			, scheduledTime(time)
			, interval(TimeSpan::zero)
			, task(std::move(task))
			, taskId(ScheduledTask::nextTaskId++)
		{
			promise = makeUnique<TaskCompletionPromise>();
		}

		ScheduledTask(TimeSpan interval, std::function<bool()> &&task)
			: initialized(true)
			, scheduledTime(Time::now())
			, interval(interval)
			, task(std::move(task))
			, taskId(ScheduledTask::nextTaskId++)
		{
			promise = makeUnique<TaskCompletionPromise>();
		}

		ScheduledTask(ScheduledTask &&other)
		{
			*this = std::move(other);
		}

		ScheduledTask &operator=(ScheduledTask &&other)
		{
			if (this != &other)
			{
				std::swap(initialized, other.initialized);
				std::swap(scheduledTime, other.scheduledTime);
				std::swap(interval, other.interval);
				task = std::move(task);
				std::swap(taskId, other.taskId);
				std::swap(promise, other.promise);
			}
			return *this;
		}

		bool initialized = false;
		SchedulerTaskId taskId = InvalidTaskId;
		Time scheduledTime;
		TimeSpan interval;
		std::function<bool()> task;
		UniquePointer<TaskCompletionPromise> promise;

		bool operator<(const ScheduledTask &rhs) const
		{
			return scheduledTime > rhs.scheduledTime;
		}

		// Returns if task should be rescheduled
		bool run()
		{
			TS_ASSERT(initialized && "Task is not properly initialized.");

			bool reschedulable = std::invoke(task) && (interval > TimeSpan::zero);

			TS_ASSERT(promise != nullptr && "My promises are empty? What?");
			promise->signalCompletion();

			return reschedulable;
		}

		static SchedulerTaskId nextTaskId;
	};

	typedef util::IterablePriorityQueue<ScheduledTask> TaskQueueType;
	TaskQueueType pendingTaskQueue;
	TaskQueueType taskQueue;

	typedef std::map<SchedulerTaskId, TaskCompletionFuture> TaskCompletionFutures;
	TaskCompletionFutures taskFutures;
	std::map<SizeType, SchedulerTaskId> workerToTaskMap;

	template <class ReturnType>
	ScheduledTaskFuture<ReturnType> scheduleOnceImpl(TimeSpan time_from_now, std::function<ReturnType()> &&f);

	SchedulerTaskId scheduleWithIntervalImpl(TimeSpan interval, std::function<bool()> &&f);

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
	TimeSpan time_from_now,
	Function &&taskFunction, Args&&... args)
{
	typedef typename std::result_of<Function(Args...)>::type ReturnType;

	return scheduleOnceImpl<ReturnType>(
		time_from_now,
		std::move(std::bind(std::forward<Function>(taskFunction), std::forward<Args>(args)...))
	);
}

template<class ReturnType, class Class, class... Args>
ScheduledTaskFuture<ReturnType> ThreadScheduler::scheduleOnce(
	TimeSpan time_from_now,
	ReturnType(Class::*taskFunction)(Args...), Class *instance, Args&&... args)
{
	return scheduleOnceImpl<ReturnType>(
		time_from_now,
		std::move(std::bind(taskFunction, instance, std::forward<Args>(args)...))
	);
}

template <class ReturnType>
ScheduledTaskFuture<ReturnType> ThreadScheduler::scheduleOnceImpl(
	TimeSpan time_from_now, std::function<ReturnType()> &&function)
{
	SharedPointer<std::packaged_task<ReturnType()>> packagedTask =
		makeShared<std::packaged_task<ReturnType()>>(function);

	ScheduledTaskFuture<ReturnType> future = packagedTask->get_future();
	{
		std::unique_lock<std::mutex> lock(queueMutex);
		if (running)
		{
			ScheduledTask task(Time::now() + time_from_now, [packagedTask]()
			{
				std::invoke(*packagedTask);
// 				(*packagedTask)();
				return false; // Schedule once task is not reschedulable
			});
			pendingTaskQueue.push(std::move(task));

			future.taskId = task.taskId;
			taskFutures.insert(std::make_pair(task.taskId, task.promise->getFuture()));
		}
	}

	schedulerCondition.notify_all();

	return future;
}

template<class Function, class... Args>
SchedulerTaskId ThreadScheduler::scheduleWithInterval(
	TimeSpan interval,
	Function &&taskFunction, Args&&... args)
{
	typedef typename std::result_of<Function(Args...)>::type ReturnType;
	static_assert(std::is_same<bool, ReturnType>::value, "Interval schedule callback should return a boolean.");

	return scheduleWithIntervalImpl(
		interval,
		std::move(std::bind(std::forward<Function>(taskFunction), std::forward<Args>(args)...))
	);
}

template<class Class, class... Args>
SchedulerTaskId ts::threading::ThreadScheduler::scheduleWithInterval(
	TimeSpan interval,
	bool(Class::*taskFunction)(Args...), Class *instance, Args&&... args)
{
	std::function<bool()> bound = std::bind(taskFunction, instance, std::forward<Args>(args)...);
	return scheduleWithIntervalImpl(
		interval,
		std::move(std::bind(taskFunction, instance, std::forward<Args>(args)...))
	);
}

inline SchedulerTaskId ThreadScheduler::scheduleWithIntervalImpl(
	TimeSpan interval, std::function<bool()> &&function)
{
	SharedPointer<std::function<bool()>> sharedTask =
		makeShared<std::function<bool()>>(std::move(function));

	SchedulerTaskId createdTaskId = InvalidTaskId;
	{
		std::unique_lock<std::mutex> lock(queueMutex);
		if (running)
		{
			ScheduledTask task(interval, [sharedTask]()
			{
				// Task return value determines if it will be rescheduled
				return std::invoke(*sharedTask);
			});
			pendingTaskQueue.push(std::move(task));

			createdTaskId = task.taskId;
			taskFutures.insert(std::make_pair(task.taskId, task.promise->getFuture()));
		}
	}

	schedulerCondition.notify_all();

	return createdTaskId;
}

TS_END_PACKAGE1()
