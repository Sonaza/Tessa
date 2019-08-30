#pragma once

#include "ts/tessa/thread/Mutex.h"
#include "ts/tessa/thread/MutexGuard.h"
#include "ts/tessa/file/FileEntry.h"
#include "ts/tessa/file/FileListStyle.h"

#include <string>
#include <vector>
#include <stack>
#include <regex>

TS_PACKAGE1(file)

class FileList : public lang::Noncopyable
{
public:
	FileList();
	FileList(const String &directoryPath, bool skipDotEntries = true, FileListStyle style = FileListStyle_All);
	~FileList();

	bool open(const String &directoryPath, bool skipDotEntries = true, FileListStyle style = FileListStyle_All);
	void close();

	bool next(FileEntry &entry);
	void rewind();

	bool isDone() const;
	void setGlobRegex(const String &pattern);

	std::vector<FileEntry> getFullListing();

private:
	mutable Mutex mutex;

	// Storing as void pointer to avoid having to include dirent.h in header
	struct DirectoryFrame
	{
		DirectoryFrame(void *ptr, const String &path)
			: ptr(ptr), rootPath(std::move(path)) {}

		void *ptr = nullptr;
		String rootPath;
	};
	std::stack<DirectoryFrame> directoryStack;
	
	String directoryPath;
	FileListStyle style = FileListStyle_All;
	bool skipDotEntries = true;
	bool done = false;

	void *globRegex = nullptr;
};

TS_END_PACKAGE1()

