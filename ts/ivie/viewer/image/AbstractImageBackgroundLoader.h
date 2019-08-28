#pragma once

#include "ts/tessa/thread/AbstractThreadEntry.h"

#include "ts/tessa/thread/ThreadScheduler.h"
#include "ts/ivie/viewer/image/Image.h"

TS_PACKAGE2(app, viewer)

class AbstractImageBackgroundLoader : public thread::AbstractThreadEntry
{
	typedef AbstractImageBackgroundLoader ThisClass;
	typedef thread::AbstractThreadEntry BaseClass;

public:
	AbstractImageBackgroundLoader(Image *ownerImage, const std::wstring &filepath);
	virtual ~AbstractImageBackgroundLoader();

	void start(bool suspendAfterBufferFull = false);
	void stop();

	void suspend(bool waitUntilBufferIsFull = false);
	void resume();

	void cancelPendingSuspension();

	bool isSuspended() const;
	virtual bool isLoadingComplete() const;

	void requestNextFrame();

	// Returns if loader was restarted (still images don't need to be)
	virtual bool restart(bool suspendAfterBufferFull = false);

	enum BackgroundLoaderState
	{
		Inactive,		// Used only before loader is started.
		Uninitialized,	// Start sets this, used to check if initialization should be done.
		Running,		// Active while loader is actively processing.
		Resuming,		// Notes when loader is resuming for onResume call.
		Suspended,		// Notes when loader is suspending for onSuspend call.
		Finished,		// Flags loader ready for uninitializing.
	};
	BackgroundLoaderState getState() const;

	std::wstring getStateString(BackgroundLoaderState state) const
	{
		switch (state)
		{
			case Inactive: return L"Inactive";
			case Uninitialized: return L"Uninitialized";
			case Running: return L"Running";
			case Resuming: return L"Resuming";
			case Suspended: return L"Suspended";
			case Finished: return L"Finished";
		}
		return L"Unknown";
	}

	virtual std::string getDebugString() const
	{
		return TS_FMT("Task ID %u", taskId);
	}

protected:
	virtual void entry() override;

	virtual bool initialize() = 0;
	virtual void deinitialize() = 0;

	virtual void onResume() = 0;
	virtual void onSuspend() = 0;

	// Returns if loader was restarted (still images don't need to be)
	virtual bool restartImpl() = 0;

	virtual bool loadNextFrame(FrameStorage &bufferStorage) = 0;
	virtual bool wasLoadingCompleted() const = 0;

	std::atomic<BackgroundLoaderState> loaderState = Inactive;
	bool suspendAfterBufferFull = false;

	std::string errorText;

	Image *ownerImage = nullptr;
	std::wstring filepath;

	bool nextFrameRequested = false;

	thread::Thread *thread = nullptr;

	mutable Mutex mutex;
	ConditionVariable condition;

// 	thread::SchedulerTaskId getTaskId() const { return taskId; }
	thread::SchedulerTaskId taskId;

private:
	thread::ThreadScheduler &threadScheduler;

};

TS_END_PACKAGE2()
