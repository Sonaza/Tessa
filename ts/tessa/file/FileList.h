#pragma once

#include "ts/tessa/thread/Mutex.h"
#include "ts/tessa/thread/MutexGuard.h"
#include "ts/tessa/file/FileEntry.h"

#include <string>
#include <vector>
#include <stack>
#include <regex>

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

enum FileListStyle : uint8
{
	FileListStyle_Directories           = priv::ListStyleBits_Directories,
	FileListStyle_Directories_Recursive = FileListStyle_Directories | priv::ListStyleBits_Recursive,

	FileListStyle_Files                 = priv::ListStyleBits_Files,
	FileListStyle_Files_Recursive       = FileListStyle_Files | priv::ListStyleBits_Recursive,

	FileListStyle_All                   = FileListStyle_Files | FileListStyle_Directories,
	FileListStyle_All_Recursive         = FileListStyle_All | priv::ListStyleBits_Recursive,

};

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
	// Storing as void pointer to avoid having to include dirent.h in header
	struct DirectoryFrame
	{
		DirectoryFrame(void *ptr, const String &absolutePath, const String &relativePath)
			: ptr(ptr)
			, absolutePath(std::move(absolutePath))
			, relativePath(std::move(relativePath))
		{}

		void *ptr = nullptr;
		String absolutePath;
		String relativePath;
	};
	std::stack<DirectoryFrame> directoryStack;
	
	String directoryPath;
	FileListStyle style = FileListStyle_All;
	bool skipDotEntries = true;
	bool done = false;

	void *globRegex = nullptr;
};

TS_END_PACKAGE1()

