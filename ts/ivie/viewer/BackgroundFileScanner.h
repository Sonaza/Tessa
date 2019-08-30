#pragma once

#include "ts/tessa/system/AbstractManagerBase.h"
#include "ts/tessa/thread/ThreadScheduler.h"

TS_PACKAGE2(app, viewer)

class BackgroundFileScanner : public system::AbstractManagerBase
{
	TS_DECLARE_MANAGER_TYPE(viewer::BackgroundFileScanner);

public:
	BackgroundFileScanner(const String &directoryPath, const std::vector<String> &allowedExtensions);
	~BackgroundFileScanner();

	virtual bool initialize();
	virtual void deinitialize();

	std::vector<String> getFileList();

	bool isExtensionAllowed(const String &filename);

	bool updateFilelist();

	lang::Signal<> filelistChangedSignal;

private:
	String directoryPath;

	std::vector<String> allowedExtensions;
	std::vector<String> filelist;

	thread::SchedulerTaskId scannerTaskId;

	Mutex mutex;
};

TS_END_PACKAGE2()
