#include "Precompiled.h"
#include "FileScanner.h"

#include "ts/tessa/file/FileListW.h"
#include "ts/tessa/file/FileUtils.h"

TS_DEFINE_MANAGER_TYPE(app::viewer::FileScanner);

TS_PACKAGE2(app, viewer)

FileScanner::FileScanner(const std::wstring &directoryPath, const std::vector<std::wstring> &allowedExtensions)
	: directoryPath(directoryPath)
	, allowedExtensions(allowedExtensions)
{
	gigaton.registerClass(this);
}

FileScanner::~FileScanner()
{
	gigaton.unregisterClass(this);
}

bool FileScanner::initialize()
{
	TS_ASSERT(!allowedExtensions.empty() && "Allowed extensions list is empty!");

	threading::ThreadScheduler &tm = getGigaton<threading::ThreadScheduler>();
	scannerTaskId = tm.scheduleWithInterval(TimeSpan::fromMilliseconds(2000), &FileScanner::scanTaskEntry, this);

	return true;
}

void FileScanner::deinitialize()
{
	threading::ThreadScheduler &tm = getGigaton<threading::ThreadScheduler>();
	tm.cancelIntervalTask(scannerTaskId);
}

std::vector<std::wstring> FileScanner::getFileList()
{
	std::lock_guard<std::mutex> lock(mutex);
	return filelist;
}

bool FileScanner::isExtensionAllowed(const std::wstring &filename)
{
	std::wstring ext = file::utils::getExtension(filename);
	return std::find(allowedExtensions.begin(), allowedExtensions.end(), ext) != allowedExtensions.end();
}

void FileScanner::updateFilelist()
{
	TS_PRINTF("FileScanner::updateFilelist\n");

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
		TS_PRINTF("  Files CHANGED!\n");
		{
			std::lock_guard<std::mutex> mg(mutex);
			filelist = std::move(templist);
		}
		filelistChangedSignal();
	}
	else
	{
		TS_PRINTF("  Files not changed!\n");
	}
}

void FileScanner::scanTaskEntry(FileScanner *scanner)
{
	TS_VERIFY_POINTERS(scanner);
	scanner->updateFilelist();
}

TS_END_PACKAGE2()


