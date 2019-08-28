#pragma once

#include "ts/tessa/system/AbstractManagerBase.h"
#include "ts/tessa/thread/ThreadScheduler.h"

TS_PACKAGE2(app, viewer)

class BackgroundFileScanner : public system::AbstractManagerBase
{
	TS_DECLARE_MANAGER_TYPE(viewer::BackgroundFileScanner);

public:
	BackgroundFileScanner(const std::wstring &directoryPath, const std::vector<std::wstring> &allowedExtensions);
	~BackgroundFileScanner();

	virtual bool initialize();
	virtual void deinitialize();

	std::vector<std::wstring> getFileList();

	bool isExtensionAllowed(const std::wstring &filename);

	bool updateFilelist();

	lang::Signal<> filelistChangedSignal;

private:
	std::wstring directoryPath;

	std::vector<std::wstring> allowedExtensions;
	std::vector<std::wstring> filelist;

	thread::SchedulerTaskId scannerTaskId;

	Mutex mutex;
};

TS_END_PACKAGE2()
