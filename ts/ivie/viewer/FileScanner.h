#pragma once

#include "ts/tessa/threading/ThreadScheduler.h"

TS_PACKAGE2(app, viewer)

class FileScanner
{
public:
	FileScanner(const std::wstring &directoryPath, const std::vector<std::wstring> &allowedExtensions);
	~FileScanner();

	std::vector<std::wstring> getFileList();

	bool isExtensionAllowed(const std::wstring &filename);

	void updateFilelist();

	static void scanTaskEntry(FileScanner *scanner);

private:
	std::wstring directoryPath;

	std::vector<std::wstring> allowedExtensions;
	std::vector<std::wstring> filelist;

	threading::SchedulerTaskId scannerTaskId;

	std::mutex mutex;
};

TS_END_PACKAGE2()
