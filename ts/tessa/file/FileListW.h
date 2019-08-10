#pragma once

#include <mutex>
#include <string>
#include <vector>
#include <stack>
#include <regex>

#include "ts/tessa/file/FileEntryW.h"
#include "ts/tessa/file/FileListStyle.h"

TS_PACKAGE1(file)

typedef std::vector<FileEntryW> FileEntryList;

class FileListW : public lang::Noncopyable
{
public:
	FileListW();
	FileListW(const std::wstring &directoryPath, bool skipDotEntries = true, FileListStyle style = FileListStyle_All);
	~FileListW();

	bool open(const std::wstring &directoryPath, bool skipDotEntries = true, FileListStyle style = FileListStyle_All);
	void close();

	bool next(FileEntryW &entry);
	void rewind();

	bool isDone() const;
	void setGlobRegex(const std::wstring &pattern);

	std::vector<FileEntryW> getFullListing();

private:
	mutable std::mutex mutex;

	// Storing as void pointer to avoid having to include dirent.h in header
	struct DirectoryFrame
	{
		DirectoryFrame(void *ptr, const std::wstring &path)
			: ptr(ptr), rootPath(std::move(path)) {}

		void *ptr = nullptr;
		std::wstring rootPath;
	};
	std::stack<DirectoryFrame> _dirStack;
	
	std::wstring _directoryPath;
	FileListStyle _style = FileListStyle_All;
	bool _skipDotEntries = true;
	bool _done = false;

	typedef std::wregex GlobRegexType;
	UniquePointer<GlobRegexType> _glob;
};

TS_END_PACKAGE1()

