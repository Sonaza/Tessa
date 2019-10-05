#pragma once

#include "ts/file/FileWatcher.h"
#include "ts/lang/common/IncludeWindows.h"

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
	virtual bool hasError() const;

	void addEvent(FileNotifyEvent &&notifyEvent);

private:
	UniquePointer<WatchData> watchData;
	std::vector<FileNotifyEvent> notifyEvents;
};

TS_END_PACKAGE1()

