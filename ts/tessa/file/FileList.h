#pragma once

#include <mutex>
#include <string>
#include <vector>
#include <stack>
#include <regex>

#include "ts/tessa/file/FileEntry.h"

TS_PACKAGE1(file)

namespace priv 
{

enum FileListStyleBits
{
	ListStyleBits_Files       = (1 << 0),
	ListStyleBits_Directories = (1 << 1),
	ListStyleBits_Recursive   = (1 << 2),
};

}

enum FileListStyle : Uint8
{
	FileListStyle_Directories           = priv::ListStyleBits_Directories,
	FileListStyle_Directories_Recursive = FileListStyle_Directories | priv::ListStyleBits_Recursive,

	FileListStyle_Files                 = priv::ListStyleBits_Files,
	FileListStyle_Files_Recursive       = FileListStyle_Files | priv::ListStyleBits_Recursive,

	FileListStyle_All                   = FileListStyle_Files | FileListStyle_Directories,
	FileListStyle_All_Recursive         = FileListStyle_All | priv::ListStyleBits_Recursive,

};

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

	UniquePointer<std::regex> _glob;
};

TS_END_PACKAGE1()

