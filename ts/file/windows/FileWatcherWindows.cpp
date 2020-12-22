#include "Precompiled.h"

#if TS_PLATFORM == TS_WINDOWS

#include "FileWatcherWindows.h"

#include "ts/file/FileWatcher.h"
#include "ts/file/FileUtils.h"

#include "ts/lang/common/IncludeWindows.h"

TS_PACKAGE1(file)

static const SizeType changeBufferSize = 3 * 1024;

struct WatchData
{
	OVERLAPPED overlapped = { 0 };
	FileWatcherWindows *watcherInst = nullptr;

	String directoryPath;
	HANDLE directoryHandle = nullptr;

	char buffer[changeBufferSize] = { 0 };

	bool recursive = false;
	SizeType flags = 0;

	bool hasError = false;
	bool stopped = false;
};

namespace
{

bool refreshWatch(WatchData *watchData, bool stopping);

void CALLBACK completionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
{
	WatchData *watchData = (WatchData*)lpOverlapped;

	if (dwErrorCode != ERROR_SUCCESS)
	{
		TS_ASSERTF(false, "error! %u", dwErrorCode);
		watchData->hasError = true;
		return;
	}

	if (dwNumberOfBytesTransfered == 0)
		return;

	FileNotifyEvent notifyEvent;

	size_t offset = 0;
	PFILE_NOTIFY_INFORMATION record = nullptr;
	do
	{
		record = (PFILE_NOTIFY_INFORMATION)&watchData->buffer[offset];
		offset += record->NextEntryOffset;

		String filename = String(
			record->FileName,
			math::min<BigSizeType>(MAX_PATH, record->FileNameLength / sizeof(wchar_t)));

		switch (record->Action)
		{
			case FILE_ACTION_ADDED:
				notifyEvent.flag = FileNotify_FileAdded;
				notifyEvent.name = std::move(filename);
			break;

			case FILE_ACTION_REMOVED:
				notifyEvent.flag = FileNotify_FileRemoved;
				notifyEvent.name = std::move(filename);
			break;

			case FILE_ACTION_MODIFIED:
				notifyEvent.flag = FileNotify_FileChanged;
				notifyEvent.name = std::move(filename);
			break;

			case FILE_ACTION_RENAMED_OLD_NAME:
				notifyEvent.flag = FileNotify_FileRenamed;
				notifyEvent.lastName = std::move(filename);
			break;

			case FILE_ACTION_RENAMED_NEW_NAME:
				TS_ASSERT(!notifyEvent.lastName.isEmpty()); // should be right after FILE_ACTION_RENAMED_OLD_NAME
				notifyEvent.flag = FileNotify_FileRenamed;
				notifyEvent.name = std::move(filename);
			break;

			default: return;
		}

		if (record->Action != FILE_ACTION_RENAMED_OLD_NAME)
		{
			TS_ASSERT(!notifyEvent.name.isEmpty());
			TS_ASSERT(notifyEvent.flag != FileNotify_FileRenamed || !notifyEvent.lastName.isEmpty());

			watchData->watcherInst->addEvent(std::move(notifyEvent));
		}

	} while (record->NextEntryOffset != 0);

	if (!watchData->stopped)
		refreshWatch(watchData, false);
}

bool refreshWatch(WatchData *watchData, bool stopping)
{
	TS_ASSERT(watchData != nullptr);
	TS_ASSERT(watchData->directoryHandle != INVALID_HANDLE_VALUE);

	return ReadDirectoryChangesW(
		watchData->directoryHandle,
		watchData->buffer, changeBufferSize,
		watchData->recursive,
		watchData->flags,
		nullptr,
		&watchData->overlapped,
		!stopping ? &completionRoutine : nullptr
	) == TRUE;
}

}

FileWatcherWindows::FileWatcherWindows(FileWatcher *parent)
	: AbstractFileWatcherImpl(parent)
{
	
}

FileWatcherWindows::~FileWatcherWindows()
{
	reset();
}

bool FileWatcherWindows::watch(const String &path, bool watchRecursive, SizeType flags)
{
	if (watchData != nullptr)
		return false;

	TS_ASSERT(flags > 0);
	if (flags == 0)
		return false;

	if (!file::exists(path) || !file::isDirectory(path))
	{
		TS_WLOG_ERROR("Watch path must exist and be a directory. Path: %s\n", path);
		return false;
	}

	TS_ASSERT(file::isAbsolutePath(path));
	if (!file::isAbsolutePath(path))
	{
		TS_WLOG_ERROR("Watch path must be absolute. Path: %s\n", path);
		return false;
	}

	watchData = makeUnique<WatchData>();

	watchData->directoryHandle = CreateFileW(
		path.toWideString().c_str(),
		FILE_LIST_DIRECTORY,
		FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
		nullptr,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
		nullptr
	);

	if (watchData->directoryHandle == INVALID_HANDLE_VALUE)
	{
		TS_LOG_ERROR("Failed to open watch directory. Path: %s\n", path);
		watchData.reset();
		return false;
	}

	watchData->watcherInst = this;

	watchData->directoryPath = path;
	watchData->recursive = watchRecursive;
	watchData->flags = flags;

	if (refreshWatch(watchData.get(), false))
		return true;

	watchData.reset();
	return false;
}

void FileWatcherWindows::reset()
{
	if (watchData == nullptr)
		return;

	watchData->stopped = true;

	CancelIo(watchData->directoryHandle);

// 	refreshWatch(watchData.get(), true);
// 
// 	if (!HasOverlappedIoCompleted(&watchData->overlapped))
// 		SleepEx(5, true);

	CloseHandle(watchData->directoryHandle);

	watchData.reset();
}

void FileWatcherWindows::update()
{
	MsgWaitForMultipleObjectsEx(0, NULL, 0, QS_ALLINPUT, MWMO_ALERTABLE);

	if (!notifyEvents.empty())
	{
		TS_ASSERT(m_parent != nullptr);
		m_parent->notifySignal(notifyEvents);
		notifyEvents.clear();
	}
}

bool FileWatcherWindows::isWatching() const
{
	return watchData != nullptr;
}

bool FileWatcherWindows::hasError() const
{
	return watchData != nullptr && watchData->hasError;
}

void FileWatcherWindows::addEvent(FileNotifyEvent &&notifyEvent)
{


	notifyEvents.push_back(std::move(notifyEvent));
}

TS_END_PACKAGE1()

#endif