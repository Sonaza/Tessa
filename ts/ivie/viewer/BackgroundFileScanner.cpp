#include "Precompiled.h"
#include "BackgroundFileScanner.h"

#include "ts/tessa/file/FileList.h"
#include "ts/tessa/file/FileUtils.h"

TS_DEFINE_MANAGER_TYPE(app::viewer::BackgroundFileScanner);

TS_PACKAGE2(app, viewer)

BackgroundFileScanner::BackgroundFileScanner(const String &directoryPath, const std::vector<String> &allowedExtensions)
	: directoryPath(directoryPath)
	, allowedExtensions(allowedExtensions)
{
	gigaton.registerClass(this);
}

BackgroundFileScanner::~BackgroundFileScanner()
{
	gigaton.unregisterClass(this);
}

bool BackgroundFileScanner::initialize()
{
	TS_ASSERT(!allowedExtensions.empty() && "Allowed extensions list is empty!");

	updateFilelist();

	thread::ThreadScheduler &tm = getGigaton<thread::ThreadScheduler>();
	scannerTaskId = tm.scheduleWithInterval(
		thread::Priority_Normal,
		TimeSpan::fromMilliseconds(2000),
		&BackgroundFileScanner::updateFilelist, this);

	return true;
}

void BackgroundFileScanner::deinitialize()
{
	thread::ThreadScheduler &tm = getGigaton<thread::ThreadScheduler>();
	tm.cancelTask(scannerTaskId);
}

std::vector<String> BackgroundFileScanner::getFileList()
{
	MutexGuard lock(mutex);
	return filelist;
}

bool BackgroundFileScanner::isExtensionAllowed(const String &filename)
{
	String ext = file::getExtension(filename);
	return std::find(allowedExtensions.begin(), allowedExtensions.end(), ext) != allowedExtensions.end();
}

bool BackgroundFileScanner::updateFilelist()
{
	file::FileList lister(directoryPath, true, file::FileListStyle_Files);
	std::vector<file::FileEntry> files = lister.getFullListing();

	std::vector<String> templist;
	templist.reserve(files.size());
	for (file::FileEntry &file : files)
	{
		if (isExtensionAllowed(file.getFilepath()))
			templist.push_back(file.getFullFilepath());
	}

	std::sort(templist.begin(), templist.end());

	bool listChanged = (templist.size() != filelist.size());
	if (!listChanged)
		listChanged = (templist != filelist);

	if (listChanged)
	{
		{
			MutexGuard lock(mutex);
			filelist = std::move(templist);
		}
		filelistChangedSignal();
	}

	return true;
}

TS_END_PACKAGE2()
