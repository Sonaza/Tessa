#pragma once

#include "ts/tessa/system/SystemManagerBase.h"

#include <thread>
#include <condition_variable>
#include <functional>
#include <memory>
#include <queue>
#include <chrono>
#include <future>

TS_PACKAGE1(threading)

typedef SizeType SchedulerTaskId;
static const SchedulerTaskId InvalidTaskId = ~0U;

class ThreadScheduler : public system::SystemManagerBase<TS_FOURCC('T','M','A','N')>
{
	TS_DECLARE_SYSTEM_MANAGER_TYPE(threading::ThreadScheduler);

public:
	ThreadScheduler(system::BaseApplication *application);
	virtual ~ThreadScheduler();

	virtual bool initialize();
	virtual void deinitialize();

	virtual void update(const sf::Time deltaTime);

	SizeType numTasks() const;
	bool hasTasks() const;

	void clearTasks();

	void cancelIntervalTask(SchedulerTaskId taskId);

	// For passing in function pointers and lambda functions
	template<class Function, class... Args>
	std::future<typename std::result_of<Function(Args...)>::type> scheduleOnce(Uint64 milliseconds_from_now, Function &&f, Args&&... args);

	// For passing in instanced class methods
	template<class ReturnType, class Class, class... Args>
	std::future<ReturnType> scheduleOnce(Uint64 milliseconds_from_now, ReturnType(Class::*taskFunction)(Args...), Class *instance, Args&&... args);

	// For passing in function pointers and lambda functions
	template<class Function, class... Args>
	SchedulerTaskId scheduleWithInterval(Uint64 interval_in_milliseconds, Function &&f, Args&&... args);
	
	static SizeType numHardwareThreads();

private:
	void createBackgroundWorkers(SizeType numWorkers);
	void destroyBackgroundWorkers();

	struct ScheduledTask
	{
		ScheduledTask() = default;

		ScheduledTask(std::chrono::system_clock::time_point time, std::function<void()> &&task)
			: time(time)
			, task(task)
			, taskId(ScheduledTask::nextTaskId++)
		{
		}

		ScheduledTask(std::chrono::system_clock::duration interval, std::function<void()> &&task)
			: time(std::chrono::system_clock::now())
			, task(task)
			, interval(interval)
			, taskId(ScheduledTask::nextTaskId++)
		{
		}

		std::chrono::system_clock::time_point time;
		std::function<void()> task;

		std::chrono::system_clock::duration interval;

		SchedulerTaskId taskId = InvalidTaskId;
		static SchedulerTaskId nextTaskId;

		bool operator<(const ScheduledTask &rhs) const
		{
			return time > rhs.time;
		}
	};

	template<class Type, class Container = std::vector<Type>>
	class IterablePriorityQueue : public std::priority_queue<Type, Container>
	{
	public:
		typedef typename Container::iterator iterator;
		typedef typename Container::const_iterator const_iterator;

		iterator begin() { return this->c.begin(); }
		iterator end() { return this->c.end(); }
		const_iterator begin() const { return this->c.begin(); }
		const_iterator end() const { return this->c.end(); }

		void clear()
		{
			IterablePriorityQueue<Type, Container> empty;
			std::swap(*this, empty);
		}
	};

	typedef IterablePriorityQueue<ScheduledTask> TaskQueueType;
	TaskQueueType pendingTaskQueue;
	TaskQueueType taskQueue;

	template <class ReturnType>
	std::future<ReturnType> _scheduleOnceImpl(Uint64 milliseconds_from_now, std::function<ReturnType()> &&f);

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
	mutable std::mutex queueMutex;
};

template<class Function, class... Args>
std::future<typename std::result_of<Function(Args...)>::type> ThreadScheduler::scheduleOnce(
	Uint64 milliseconds_from_now,
	Function &&taskFunction, Args&&... args)
{
	typedef typename std::result_of<Function(Args...)>::type ReturnType;

	std::function<ReturnType()> bound = std::bind(std::forward<Function>(taskFunction), std::forward<Args>(args)...);
	return _scheduleOnceImpl<ReturnType>(milliseconds_from_now, std::move(bound));
}

template<class ReturnType, class Class, class... Args>
std::future<ReturnType> ThreadScheduler::scheduleOnce(
	Uint64 milliseconds_from_now,
	ReturnType(Class::*taskFunction)(Args...), Class *instance, Args&&... args)
{
	std::function<ReturnType()> bound = std::bind(taskFunction, instance, std::forward<Args>(args)...);
	return _scheduleOnceImpl<ReturnType>(milliseconds_from_now, std::move(bound));
}

template <class ReturnType>
std::future<ReturnType> ThreadScheduler::_scheduleOnceImpl(Uint64 milliseconds_from_now, std::function<ReturnType()> &&bound)
{
	SharedPointer<std::packaged_task<ReturnType()>> packagedTask = makeShared<std::packaged_task<ReturnType()>>(bound);

	std::future<ReturnType> result = packagedTask->get_future();
	{
		std::unique_lock<std::mutex> lock(queueMutex);
		if (running)
		{
			std::chrono::system_clock::time_point scheduledTime = std::chrono::system_clock::now() + std::chrono::milliseconds(milliseconds_from_now);
			ScheduledTask task(scheduledTime, [packagedTask]()
			{
				(*packagedTask)();
			});
			pendingTaskQueue.push(std::move(task));
		}
	}

	schedulerCondition.notify_all();

	return result;
}

template<class Function, class... Args>
SchedulerTaskId ThreadScheduler::scheduleWithInterval(
	Uint64 interval_in_milliseconds,
	Function &&taskFunction, Args&&... args)
{
	typedef typename std::result_of<Function(Args...)>::type ReturnType;

	SharedPointer<std::function<ReturnType()>> sharedTask = makeShared<std::function<ReturnType()>>();
	*sharedTask = std::bind(std::forward<Function>(taskFunction), std::forward<Args>(args)...);
	
// 	SharedPointer<std::packaged_task<ReturnType()>> packagedTask = makeShared<std::packaged_task<ReturnType()>>(bound);

	SchedulerTaskId createdTaskId = InvalidTaskId;

	{
		std::unique_lock<std::mutex> lock(queueMutex);
		if (running)
		{
			std::chrono::system_clock::duration interval = std::chrono::milliseconds(interval_in_milliseconds);
			ScheduledTask task(interval, [sharedTask]()
			{
				(*sharedTask)();
// 				(*packagedTask)();
			});
			pendingTaskQueue.push(std::move(task));

			createdTaskId = task.taskId;
		}
	}

	schedulerCondition.notify_all();

	return createdTaskId;
}

TS_END_PACKAGE1()
