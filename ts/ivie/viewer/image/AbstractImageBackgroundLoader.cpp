#include "Precompiled.h"
#include "AbstractImageBackgroundLoader.h"

#include "ts/tessa/threading/Thread.h"

TS_PACKAGE2(app, viewer)

AbstractImageBackgroundLoader::AbstractImageBackgroundLoader(Image *ownerImage, const std::wstring &filepath)
	: ownerImage(ownerImage)
	, filepath(filepath)
	, threadScheduler(TS_GET_GIGATON().getGigaton<threading::ThreadScheduler>())
{
	TS_ASSERT(ownerImage != nullptr);
}

AbstractImageBackgroundLoader::~AbstractImageBackgroundLoader()
{

}

void AbstractImageBackgroundLoader::start(bool suspendAfterBufferFullParam)
{
	TS_ASSERT(loaderState == Inactive);
	if (loaderState != Inactive)
		return;

	suspendAfterBufferFull = suspendAfterBufferFullParam;

	loaderState = Uninitialized;
	taskId = threadScheduler.scheduleThreadEntry(this);
	TS_WPRINTF("AbstractImageBackgroundLoader::start() : Task ID %u [%s]\n", taskId, filepath);
}

void AbstractImageBackgroundLoader::stop()
{
	TS_WPRINTF("AbstractImageBackgroundLoader::stop()  : Task ID %u [%s]\n", taskId, filepath);

	{
		std::unique_lock<std::mutex> lock(mutex);
		if (loaderState == Suspended)
		{
			deinitialize();
			return;
		}

		loaderState = Finished;
	}
	condition.notify_all();

	threadScheduler.waitUntilTaskComplete(taskId);
}

void AbstractImageBackgroundLoader::suspend(bool waitUntilBufferIsFull)
{
	{
		std::unique_lock<std::mutex> lock(mutex);
		if (loaderState != Running)
			return;

		if (waitUntilBufferIsFull)
		{
			suspendAfterBufferFull = true;
			return;
		}

		loaderState = Suspended;
	}
	condition.notify_all();

	threadScheduler.waitUntilTaskComplete(taskId);
}

void AbstractImageBackgroundLoader::resume()
{
	std::unique_lock<std::mutex> lock(mutex);
	if (loaderState == Suspended)
	{
		suspendAfterBufferFull = false;
		loaderState = Resuming;
		taskId = threadScheduler.scheduleThreadEntry(this);
	}
}

bool AbstractImageBackgroundLoader::isSuspended() const
{
	std::unique_lock<std::mutex> lock(mutex);
	return loaderState == Suspended;
}

bool AbstractImageBackgroundLoader::isLoadingComplete() const
{
	std::unique_lock<std::mutex> lock(mutex);
	return loaderState == Finished;
}

void AbstractImageBackgroundLoader::entry()
{
	TS_WPRINTF("AbstractImageBackgroundLoader::entry() starting : TaskID %u [%s]\n", taskId, filepath);

	{
		std::unique_lock<std::mutex> lock(mutex);
		if (loaderState == Uninitialized)
		{
			if (initialize())
			{
				loaderState = Running;
			}
			else
			{
				TS_LOG_ERROR("AbstractImageBackgroundLoader failed to initialize.");

				loaderState = Finished;
				ownerImage->loaderState = Image::Error;
				return;
			}
		}
		else if (loaderState == Resuming)
		{
			TS_PRINTF("Loader is resuming\n");
			onResume();
			loaderState = Running;
			ownerImage->loaderState = Image::Loading;
		}
		else
		{
			TS_ASSERT(!"This shouldn't ever happen.");
		}
	}

	enum ProcessingState
	{
		Pending,
		Complete,
		Error,
		Aborted,
	};
	ProcessingState processingState = Pending;

	while (loaderState)
	{
		while (!ownerImage->frameBuffer.isFull())
		{
			nextFrameRequested = false;

			FrameStorage &storage = ownerImage->frameBuffer.getWritePtr();
			bool success = loadNextFrame(storage);
			if (success)
			{
				ownerImage->frameBuffer.incrementWrite();

				if (isLoadingComplete())
				{
					ownerImage->frameBuffer.removeReadConstraint();
					processingState = Complete;
				}
			}
			else
			{
				processingState = Error;
			}

			if (processingState != Pending)
			{
				loaderState = Finished;
				break;
			}
			Thread::sleep(TimeSpan::fromMilliseconds(4));
		}
		if (suspendAfterBufferFull && loaderState == Running)
		{
			TS_WPRINTF("Suspending loader since buffer is full : TaskID %u [%s]\n", taskId, filepath);
			loaderState = Suspended;
			break;
		}

		if (loaderState != Running)
			break;
		
		std::unique_lock<std::mutex> lock(mutex);
		condition.wait(lock, [&]()
		{
			return loaderState != Running || nextFrameRequested;
		});
// 		Thread::sleep(TimeSpan::fromMilliseconds(5));
	}

	if (loaderState != Suspended && processingState == Pending)
		processingState = Aborted;

	switch (processingState)
	{
		case Aborted:
		case Complete: ownerImage->loaderState = Image::Complete; break;
		case Error:    ownerImage->loaderState = Image::Error; break;
		case Pending:
			if (loaderState == Suspended)
			{
				ownerImage->loaderState = Image::Suspended;
			}
		break;
		default: /* Nada */ break;
	}

	if (loaderState == Suspended)
	{
		onSuspend();
	}
	else if (loaderState == Finished)
	{
		deinitialize();
	}
	else
	{
		TS_ASSERT(!"This shouldn't ever happen.");
	}

	TS_WPRINTF("AbstractImageBackgroundLoader::entry() exiting with state '%s' : TaskID %u [%s]\n", getStateString(loaderState), taskId, filepath);

	TS_ASSERTF(
		ownerImage->loaderState == Image::Complete ||
		ownerImage->loaderState == Image::Suspended ||
		ownerImage->loaderState == Image::Error,
		"OwnerImage's state at exit should always be Complete, Suspended or Error."
	);
}

void AbstractImageBackgroundLoader::requestNextFrame()
{
	{
		std::unique_lock<std::mutex> lock(mutex);
		nextFrameRequested = true;
	}
	condition.notify_one();
}

bool AbstractImageBackgroundLoader::restart(bool suspendAfterBufferFullParam)
{
	if (restartImpl())
	{
		{
			std::unique_lock<std::mutex> lock(mutex);
			nextFrameRequested = true;
			suspendAfterBufferFull = suspendAfterBufferFullParam;
		}
		condition.notify_one();
		return true;
	}
	return false;
}

AbstractImageBackgroundLoader::BackgroundLoaderState AbstractImageBackgroundLoader::getState() const
{
	std::unique_lock<std::mutex> lock(mutex);
	return loaderState;
}

TS_END_PACKAGE2()
