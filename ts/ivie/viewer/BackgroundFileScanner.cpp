#include "Precompiled.h"
#include "BackgroundFileScanner.h"

#include "ts/tessa/file/FileListW.h"
#include "ts/tessa/file/FileUtils.h"

TS_DEFINE_MANAGER_TYPE(app::viewer::BackgroundFileScanner);

TS_PACKAGE2(app, viewer)

BackgroundFileScanner::BackgroundFileScanner(const std::wstring &directoryPath, const std::vector<std::wstring> &allowedExtensions)
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

	threading::ThreadScheduler &tm = getGigaton<threading::ThreadScheduler>();
	scannerTaskId = tm.scheduleWithInterval(TimeSpan::fromMilliseconds(2000), &BackgroundFileScanner::updateFilelist, this);

	return true;
}

void BackgroundFileScanner::deinitialize()
{
	threading::ThreadScheduler &tm = getGigaton<threading::ThreadScheduler>();
	tm.cancelTask(scannerTaskId);
}

std::vector<std::wstring> BackgroundFileScanner::getFileList()
{
	std::lock_guard<std::mutex> lock(mutex);
	return filelist;
}

bool BackgroundFileScanner::isExtensionAllowed(const std::wstring &filename)
{
	std::wstring ext = file::utils::getExtension(filename);
	return std::find(allowedExtensions.begin(), allowedExtensions.end(), ext) != allowedExtensions.end();
}

bool BackgroundFileScanner::updateFilelist()
{
	file::FileListW lister(directoryPath, true, file::FileListStyle_Files);
	std::vector<file::FileEntryW> files = lister.getFullListing();

	std::vector<std::wstring> templist;
	templist.reserve(files.size());
	for (file::FileEntryW &file : files)
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
			std::lock_guard<std::mutex> mg(mutex);
			filelist = std::move(templist);
		}
		filelistChangedSignal();
	}

	return true;
}

TS_END_PACKAGE2()


