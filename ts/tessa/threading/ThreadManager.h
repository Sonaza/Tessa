#pragma once

#include "ts/tessa/system/SystemManagerBase.h"

#include <thread>
#include <condition_variable>
#include <functional>
#include <memory>
#include <queue>
#include <future>

TS_PACKAGE1(threading)

enum TaskPriority
{
	TaskPriorityHigh,
	TaskPriorityNormal,
	TaskPriorityLow,
};

class ThreadManager : public system::SystemManagerBase<TS_FOURCC('T','M','A','N')>
{
	TS_DECLARE_SYSTEM_MANAGER_TYPE(threading::ThreadManager);

public:
	ThreadManager(system::Application *application);
	virtual ~ThreadManager();

	virtual bool initialize();
	virtual void deinitialize();

	virtual void update(const sf::Time deltaTime);

	SizeType numTasks() const;
	bool hasTasks() const;

	void clearTasks();

	// For passing in function pointers and lambda functions
	template<class Function, class... Args>
	std::future<typename std::result_of<Function(Args...)>::type> push(TaskPriority priority, Function &&f, Args&&... args);

	// For passing in instanced class methods
	template<class ReturnType, class Class, class... Args>
	std::future<ReturnType> push(TaskPriority priority, ReturnType(Class::*taskFunction)(Args...), Class *instance, Args&&... args);

	static SizeType numHardwareThreads();

private:
	void createPoolThreads(SizeType numThreads);
	void destroyPoolThreads();

	template <class ReturnType>
	std::future<ReturnType> _pushImpl(TaskPriority priority, std::function<ReturnType()> &&f);

	void _threadTaskRunnerImpl(SizeType threadIndex);

	bool running = false;

	std::condition_variable condition;
	mutable std::mutex queueMutex;

	std::vector<UniquePointer<std::thread>> workerThreads;

	struct TaskContainer
	{
		TaskContainer() = default;
		TaskContainer(TaskPriority priority, std::function<void()> &&task)
			: priority(priority)
			, task(task)
		{
		}

		TaskPriority priority;
		std::function<void()> task;

		bool operator()(const TaskContainer &a, const TaskContainer &b) const
		{
			return a.priority > b.priority;
		}
	};
	typedef std::priority_queue<TaskContainer, std::vector<TaskContainer>, TaskContainer> TaskQueueType;
	TaskQueueType taskQueue;
};

template<class Function, class... Args>
std::future<typename std::result_of<Function(Args...)>::type> ThreadManager::push(TaskPriority priority, Function &&taskFunction, Args&&... args)
{
	typedef typename std::result_of<Function(Args...)>::type ReturnType;

	std::function<ReturnType()> bound = std::bind(std::forward<Function>(taskFunction), std::forward<Args>(args)...);
	return _pushImpl<ReturnType>(priority, std::move(bound));
}

template<class ReturnType, class Class, class... Args>
std::future<ReturnType> ThreadManager::push(TaskPriority priority, ReturnType(Class::*taskFunction)(Args...), Class *instance, Args&&... args)
{
	std::function<ReturnType()> bound = std::bind(taskFunction, instance, std::forward<Args>(args)...);
	return _pushImpl<ReturnType>(priority, std::move(bound));
}

template <class ReturnType>
std::future<ReturnType> ThreadManager::_pushImpl(TaskPriority priority, std::function<ReturnType()> &&bound)
{
	SharedPointer<std::packaged_task<ReturnType()>> packagedTask = makeShared<std::packaged_task<ReturnType()>>(bound);

	std::future<ReturnType> result = packagedTask->get_future();
	{
		std::unique_lock<std::mutex> lock(queueMutex);
		if (running)
		{
			taskQueue.emplace(priority, [packagedTask]()
			{
				(*packagedTask)();
			});
		}
	}

	condition.notify_one();

	return result;
}

TS_END_PACKAGE1()
