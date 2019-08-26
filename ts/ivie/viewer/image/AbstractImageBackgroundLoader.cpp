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
	taskId = threadScheduler.scheduleThreadEntry(this, threading::Priority_Normal);
	TS_WPRINTF("AbstractImageBackgroundLoader::start() : Task ID %u [%s]\n", taskId, filepath);
}

void AbstractImageBackgroundLoader::stop()
{
	TS_WPRINTF("AbstractImageBackgroundLoader::stop()  : Task ID %u [%s]\n", taskId, filepath);

	if (loaderState == Suspended)
	{
		deinitialize();
		return;
	}

	{
		std::unique_lock<std::mutex> lock(mutex);
		loaderState = Finished;
	}
	condition.notify_all();
	threadScheduler.waitUntilTaskComplete(taskId);
}

void AbstractImageBackgroundLoader::suspend(bool waitUntilBufferIsFull)
{
	{
		if (loaderState != Running)
			return;

		std::unique_lock<std::mutex> lock(mutex);
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
	if (loaderState == Suspended)
	{
		std::unique_lock<std::mutex> lock(mutex);
		suspendAfterBufferFull = false;
		loaderState = Resuming;
		taskId = threadScheduler.scheduleThreadEntry(this, threading::Priority_High);
	}
}

void AbstractImageBackgroundLoader::cancelPendingSuspension()
{
	std::unique_lock<std::mutex> lock(mutex);
	suspendAfterBufferFull = false;
}

bool AbstractImageBackgroundLoader::isSuspended() const
{
	return loaderState == Suspended;
}

bool AbstractImageBackgroundLoader::isLoadingComplete() const
{
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
				ownerImage->setState(Image::Error);
				return;
			}
		}
		else if (loaderState == Resuming)
		{
			TS_WPRINTF("Loader is resuming : TaskID %u [%s]\n", taskId, filepath);
			onResume();
			loaderState = Running;
			ownerImage->setState(Image::Loading);
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

	while (loaderState == Running)
	{
		while (!ownerImage->getIsBufferFull() && loaderState == Running)
		{
			nextFrameRequested = false;

			if (ownerImage->isUnloading())
			{
				processingState = Aborted;
				break;
			}

			try
			{
				FrameStorage &storage = ownerImage->getNextBuffer();
				bool success = loadNextFrame(storage);

				if (success)
				{
					ownerImage->swapBuffer();

					if (wasLoadingCompleted())
					{
						ownerImage->finalizeBuffer();
						processingState = Complete;
					}
				}
				else
				{
					processingState = Error;
				}
			}
			catch (const ImageUnloadingException &e)
			{
				TS_PRINTF("Task ID %u : loading was interrupted, image is being unloaded.\n", taskId);
				TS_UNUSED_VARIABLE(e);
				processingState = Aborted;
			}

			if (loaderState != Running)
				break;

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
			TS_WPRINTF("OwnerImage state %s : Task ID %u\n", ownerImage->getStateString(ownerImage->loaderState), taskId);
		break;
		case Complete: ownerImage->setState(Image::Complete); break;
		case Error:    ownerImage->setState(Image::Error); break;
		case Pending:
			if (loaderState == Suspended)
			{
				ownerImage->setState(Image::Suspended);
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
		ownerImage->getState() == Image::Complete ||
		ownerImage->getState() == Image::Unloading ||
		ownerImage->getState() == Image::Suspended ||
		ownerImage->getState() == Image::Error,
		"OwnerImage's state at exit should always be Complete, Unloading, Suspended or Error."
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
	return loaderState;
}

TS_END_PACKAGE2()