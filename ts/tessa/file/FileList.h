#pragma once

#include <mutex>
#include <string>
#include <vector>
#include <stack>
#include <regex>

#include "ts/tessa/file/FileEntry.h"
#include "ts/tessa/file/FileListStyle.h"

TS_PACKAGE1(file)

typedef std::vector<FileEntry> FileEntryList;

class FileList : public lang::Noncopyable
{
public:
	FileList();
	FileList(const std::string &directoryPath, bool skipDotEntries = true, FileListStyle style = FileListStyle_All);
	~FileList();

	bool open(const std::string &directoryPath, bool skipDotEntries = true, FileListStyle style = FileListStyle_All);
	void close();

	bool next(FileEntry &entry);
	void rewind();

	bool isDone() const;
	void setGlobRegex(const std::string &pattern);

	std::vector<FileEntry> getFullListing();

private:
	mutable std::mutex mutex;

	// Storing as void pointer to avoid having to include dirent.h in header
	struct DirectoryFrame
	{
		DirectoryFrame(void *ptr, const std::string &path)
			: ptr(ptr), rootPath(std::move(path)) {}

		void *ptr = nullptr;
		std::string rootPath;
	};
	std::stack<DirectoryFrame> _dirStack;
	
	std::string _directoryPath;
	FileListStyle _style = FileListStyle_All;
	bool _skipDotEntries = true;
	bool _done = false;

	typedef std::regex GlobRegexType;
	UniquePointer<GlobRegexType> _glob;
};

TS_END_PACKAGE1()

