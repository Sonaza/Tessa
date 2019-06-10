#pragma once

#include <thread>
#include <condition_variable>
#include <functional>
#include <memory>
#include <queue>
#include <future>

TS_PACKAGE1(system)

class ThreadPool
{
public:
	ThreadPool(SizeType poolNumThreads);
	virtual ~ThreadPool();

	enum TaskPriority
	{
		High,
		Normal,
		Low,
	};

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
	template <class ReturnType>
	std::future<ReturnType> pushImpl(TaskPriority priority, std::function<ReturnType()> &&f);

	void threadTaskRunnerImpl(SizeType threadIndex);

	bool running = true;

	std::condition_variable condition;
	mutable std::mutex queueMutex;

	std::vector<std::unique_ptr<std::thread>> workerThreads;

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
std::future<typename std::result_of<Function(Args...)>::type> ThreadPool::push(TaskPriority priority, Function &&taskFunction, Args&&... args)
{
	typedef typename std::result_of<Function(Args...)>::type ReturnType;

	std::function<ReturnType()> bound = std::bind(std::forward<Function>(taskFunction), std::forward<Args>(args)...);
	return pushImpl<ReturnType>(priority, std::move(bound));
}

template<class ReturnType, class Class, class... Args>
std::future<ReturnType> ThreadPool::push(TaskPriority priority, ReturnType(Class::*taskFunction)(Args...), Class *instance, Args&&... args)
{
	std::function<ReturnType()> bound = std::bind(taskFunction, instance, std::forward<Args>(args)...);
	return pushImpl<ReturnType>(priority, std::move(bound));
}

template <class ReturnType>
std::future<ReturnType> ThreadPool::pushImpl(TaskPriority priority, std::function<ReturnType()> &&bound)
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
