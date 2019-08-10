#include "Precompiled.h"
#include "FileScanner.h"

#include "ts/ivie/util/NaturalSort.h"
#include "ts/tessa/file/FileListW.h"
#include "ts/tessa/file/FileUtils.h"

#include "ts/tessa/threading/ThreadScheduler.h"

TS_PACKAGE2(app, viewer)

FileScanner::FileScanner(const std::wstring &directoryPath, const std::vector<std::wstring> &allowedExtensions)
	: directoryPath(directoryPath)
	, allowedExtensions(allowedExtensions)
{
	TS_GIGATON_REGISTER_CLASS(this);

	threading::ThreadScheduler &tm = TS_GET_GIGATON().getGigaton<threading::ThreadScheduler>();
	scannerTaskId = tm.scheduleWithInterval(2000, &FileScanner::scanTaskEntry, this);
}

FileScanner::~FileScanner()
{
	TS_GIGATON_UNREGISTER_CLASS(this);

	threading::ThreadScheduler &tm = TS_GET_GIGATON().getGigaton<threading::ThreadScheduler>();
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
	file::FileListW lister(directoryPath, true, file::FileListStyle_Files);
	std::vector<file::FileEntryW> files = lister.getFullListing();

	TS_PRINTF("Updating filelist!\n");

	std::vector<std::wstring> tempList;
	tempList.reserve(files.size());
	for (file::FileEntryW &file : files)
	{
		if (isExtensionAllowed(file.getFilepath()))
		{
			TS_WPRINTF("  %s\n", file.getFullFilepath());
			tempList.push_back(file.getFullFilepath());
		}
	}

	std::sort(tempList.begin(), tempList.end(), util::NaturalSortByExtension);

	{
		std::lock_guard<std::mutex> mg(mutex);
		filelist = std::move(tempList);
	}

	TS_PRINTF("Donezo!\n");
}

void FileScanner::scanTaskEntry(FileScanner *scanner)
{
	TS_VERIFY_POINTERS(scanner);
	scanner->updateFilelist();
}

TS_END_PACKAGE2()


