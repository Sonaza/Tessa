#pragma once

TS_PACKAGE1(file)

enum FileWatchFlags : SizeType
{
	FileWatch_FileChanges        = (1 << 0),
	FileWatch_DirectoryChanges   = (1 << 1),
	FileWatch_AttributeChanges   = (1 << 2),
	FileWatch_SizeChanges        = (1 << 3),
	FileWatch_LastWriteChanges   = (1 << 4),
};

enum FileNotifyEventFlag : SizeType
{
	FileNotify_FileAdded,
	FileNotify_FileRemoved,
	FileNotify_FileChanged,
	FileNotify_FileRenamed,
};

struct FileNotifyEvent
{
	FileNotifyEventFlag flag;
	String name;
	String lastName;
};

class FileWatcher;

class AbstractFileWatcherImpl
{
public:
	AbstractFileWatcherImpl(FileWatcher *parent)
		: m_parent(parent)
	{}

	virtual ~AbstractFileWatcherImpl() = default;

	virtual bool watch(const String &path, bool watchRecursive, SizeType flags) = 0;
	virtual void reset() = 0;

	virtual void update() = 0;

	virtual bool isWatching() const = 0;
	virtual bool hasError() const = 0;

protected:
	FileWatcher *m_parent = nullptr;
};

class FileWatcher
{
public:
	FileWatcher();
	FileWatcher(const String &path, bool watchRecursive, SizeType flags);

	bool watch(const String &path, bool watchRecursive, SizeType flags);
	void reset();

	void update();

	bool isWatching() const;
	bool hasError() const;

	lang::Signal<const std::vector<FileNotifyEvent>&> notifySignal;

private:
	ScopedPointer<AbstractFileWatcherImpl> m_impl;
};

TS_END_PACKAGE1()

