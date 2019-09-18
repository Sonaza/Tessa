#pragma once

#ifndef TS_INCLUDE_FILEWATCH_IMPL
#error "The special impl headers are only supposed to be included in FileWatcher.cpp"
#endif

#include "ts/tessa/file/FileWatcher.h"
#include "ts/tessa/common/IncludeWindows.h"

TS_PACKAGE1(file)

struct WatchData;

class FileWatcherWindows : public AbstractFileWatcherImpl
{
public:
	FileWatcherWindows(FileWatcher *parent);
	virtual ~FileWatcherWindows();

	virtual bool watch(const String &path, bool watchRecursive, SizeType flags);
	virtual void reset();

	virtual void update();

	virtual bool isWatching() const;

	void addEvent(FileNotifyEvent &&notifyEvent);

private:
	UniquePointer<WatchData> watchData;
	std::vector<FileNotifyEvent> notifyEvents;
};

TS_END_PACKAGE1()

