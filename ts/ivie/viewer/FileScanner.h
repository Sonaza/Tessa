#pragma once

#include "ts/tessa/system/AbstractManagerBase.h"
#include "ts/tessa/threading/ThreadScheduler.h"

#include "ts/tessa/lang/Signal.h"

TS_PACKAGE2(app, viewer)

class FileScanner : public system::AbstractManagerBase
{
	TS_DECLARE_MANAGER_TYPE(viewer::FileScanner);

public:
	FileScanner(const std::wstring &directoryPath, const std::vector<std::wstring> &allowedExtensions);
	~FileScanner();

	virtual bool initialize();
	virtual void deinitialize();

	std::vector<std::wstring> getFileList();

	bool isExtensionAllowed(const std::wstring &filename);

	void updateFilelist();

	static void scanTaskEntry(FileScanner *scanner);

	lang::Signal<> filelistChangedSignal;

private:
	std::wstring directoryPath;

	std::vector<std::wstring> allowedExtensions;
	std::vector<std::wstring> filelist;

	threading::SchedulerTaskId scannerTaskId;

	std::mutex mutex;
};

TS_END_PACKAGE2()
