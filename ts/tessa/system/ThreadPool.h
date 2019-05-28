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

	template<class TaskFunction, class... TaskArgs>
	std::future<typename std::result_of<TaskFunction(TaskArgs...)>::type> push(TaskPriority priority, TaskFunction && f, TaskArgs&&... args);

	static SizeType numHardwareThreads()
	{
		return std::thread::hardware_concurrency();
	}

private:
	void threadTaskRunnerImpl(SizeType threadIndex);

	bool running = true;

	std::condition_variable condition;
	mutable std::mutex queueMutex;

	std::vector<std::unique_ptr<std::thread>> workerThreads;

	struct TaskContainer
	{
		std::function<void()> task;
		TaskPriority priority;

		bool operator()(const TaskContainer &a, const TaskContainer &b) const
		{
			return a.priority > b.priority;
		}
	};
	typedef std::priority_queue<TaskContainer, std::vector<TaskContainer>, TaskContainer> TaskQueueType;
	TaskQueueType taskQueue;
};

template<class TaskFunction, class... TaskArgs>
std::future<typename std::result_of<TaskFunction(TaskArgs...)>::type> ThreadPool::push(TaskPriority priority, TaskFunction && taskFunction, TaskArgs&&... args)
{
	typedef typename std::result_of<TaskFunction(TaskArgs...)>::type ReturnType;

	std::function<void()> boundTaskFunction = std::bind(std::forward<TaskFunction>(taskFunction), std::forward<TaskArgs>(args)...);
	std::shared_ptr<std::packaged_task<ReturnType()>> packagedTask = std::make_shared<std::packaged_task<ReturnType()>>(boundTaskFunction);

	std::future<ReturnType> result = packagedTask->get_future();
	{
		std::unique_lock<std::mutex> lock(queueMutex);
		if (running)
		{
			TaskContainer task;
			task.task = [packagedTask]()
			{
				(*packagedTask)();
			};
			task.priority = priority;
			taskQueue.emplace(std::move(task));
		}
	}

	condition.notify_one();

	return result;
}

TS_END_PACKAGE1()
