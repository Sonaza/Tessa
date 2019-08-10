#include "Precompiled.h"
#include "FileScanner.h"

#include "ts/ivie/util/NaturalSort.h"
#include "ts/tessa/file/FileListW.h"
#include "ts/tessa/file/FileUtils.h"

#include "ts/tessa/threading/Thread.h"
#include "ts/tessa/threading/BaseThreadEntry.h"

TS_PACKAGE2(app, viewer)

class FileScanner::ScannerBackgroundTask : public threading::BaseThreadEntry
{
public:
	ScannerBackgroundTask(const std::wstring &directoryPath, const std::vector<std::wstring> &allowedExtensions)
		: directoryPath(directoryPath)
		, allowedExtensions(allowedExtensions)
	{
		thread = Thread::createThread(this, "FileScanner Background Task");
	}

	virtual ~ScannerBackgroundTask()
	{
		if (thread != nullptr)
			Thread::joinThread(thread);
	}

	virtual void entry()
	{
		while (running)
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

			TS_PRINTF("Donezo!\n\n");

			std::unique_lock<std::mutex> lock(waiterMutex);
			condition.wait_for(lock, std::chrono::milliseconds(2000), [&]()
			{
				return !running;
			});

// 			Thread::sleep(500);
		}
	}

	bool isExtensionAllowed(const std::wstring &filename)
	{
		std::wstring ext = file::utils::getExtension(filename);
		return std::find(allowedExtensions.begin(), allowedExtensions.end(), ext) != allowedExtensions.end();
	}

	std::vector<std::wstring> getFileList()
	{
		std::lock_guard<std::mutex> mg(mutex);
		return filelist;
	}

	void stop()
	{
		std::lock_guard<std::mutex> mg(mutex);
		running = false;
		condition.notify_all();
	}

private:
	std::wstring directoryPath;

	std::vector<std::wstring> allowedExtensions;
	std::vector<std::wstring> filelist;

	bool running = true;

	std::mutex mutex;
	Thread *thread = nullptr;

	std::condition_variable condition;
	std::mutex waiterMutex;
};

FileScanner::FileScanner(const std::wstring &directoryPath, const std::vector<std::wstring> &allowedExtensions)
{
	TS_GIGATON_REGISTER_CLASS(this);

	backgroundTask.reset(new ScannerBackgroundTask(directoryPath, allowedExtensions));
}

FileScanner::~FileScanner()
{
	TS_GIGATON_UNREGISTER_CLASS(this);

	if (backgroundTask != nullptr)
	{
		backgroundTask->stop();
		backgroundTask.reset();
	}
}

std::vector<std::wstring> FileScanner::getFileList()
{
	TS_VERIFY_POINTERS_WITH_RETURN_VALUE(std::vector<std::wstring>(), backgroundTask);
	return backgroundTask->getFileList();
}

TS_END_PACKAGE2()