#include "Precompiled.h"
#include "ts/tessa/threading/ThreadManager.h"
#include "ts/tessa/threading/ThreadUtils.h"

#include "ts/tessa/Config.h"

TS_DEFINE_SYSTEM_MANAGER_TYPE(threading::ThreadManager);

TS_PACKAGE1(threading)

ThreadManager::ThreadManager(system::Application *application)
	: SystemManagerBase(application)
{
	TS_GIGATON_REGISTER_CLASS(this);
}

ThreadManager::~ThreadManager()
{
	TS_GIGATON_UNREGISTER_CLASS(this);
}

bool ThreadManager::initialize()
{
	const SizeType numThreads = math::min(MAX_THREAD_POOL_THREAD_COUNT, ThreadManager::numHardwareThreads());
	createPoolThreads(numThreads);
	
	return true;
}

void ThreadManager::deinitialize()
{
	destroyPoolThreads();
}

void ThreadManager::update(const sf::Time deltaTime)
{

}

void ThreadManager::createPoolThreads(SizeType poolNumThreads)
{
	{
		std::unique_lock<std::mutex> lock(queueMutex);
		TS_ASSERT(running == false && "Trying to create pool threads again even though it is already running.");
		running = true;
	}
	for (SizeType i = 0; i < poolNumThreads; ++i)
	{
		UniquePointer<std::thread> thread = makeUnique<std::thread>(&ThreadManager::_threadTaskRunnerImpl, this, i);
		
		utils::setThreadName(*thread, TS_FMT("ThreadManager Worker %u", i));
		utils::setThreadPriority(*thread, threading::utils::ThreadPriority_High);

		workerThreads.push_back(std::move(thread));
	}
}

void ThreadManager::destroyPoolThreads()
{
	{
		std::unique_lock<std::mutex> lock(queueMutex);
		running = false;
	}

	condition.notify_all();

	for (SizeType i = 0; i < workerThreads.size(); ++i)
	{
		workerThreads[i]->join();
	}
}

SizeType ThreadManager::numTasks() const
{
	std::unique_lock<std::mutex> lock(queueMutex);
	return (SizeType)taskQueue.size();
}

bool ThreadManager::hasTasks() const
{
	std::unique_lock<std::mutex> lock(queueMutex);
	return !taskQueue.empty();
}

void ThreadManager::clearTasks()
{
	{
		std::unique_lock<std::mutex> lock(queueMutex);
		TaskQueueType empty;
		std::swap(taskQueue, empty);
	}
	condition.notify_all();
}

SizeType ThreadManager::numHardwareThreads()
{
	return (SizeType)std::thread::hardware_concurrency();
}

void ThreadManager::_threadTaskRunnerImpl(SizeType threadIndex)
{
	while (true)
	{
		TaskContainer task;
		{
			std::unique_lock<std::mutex> lock(queueMutex);
			condition.wait(lock, [this]()
			{
				return !running || !taskQueue.empty();
			});

			if (!running && taskQueue.empty())
				return;

			task = std::move(taskQueue.top());
			taskQueue.pop();
		}

		task.task();
	}
}

TS_END_PACKAGE1()
