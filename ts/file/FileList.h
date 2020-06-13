#pragma once

#include "ts/file/FileListEntry.h"

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

enum FileListFlags
{
	// Windows only: Uses large fetch. Can more optimal when scanning whole directory contents.
	FileListFlags_LargeFetch         = (1 << 0),

	// File list omits setting the root path in FileListEntry.
	// Can be more optimal when root path is not required.
	FileListFlags_ExcludeRootPath    = (1 << 1),

	// File list sets FileListEntry's file name to be the file name only,
	// excluding the relative path section to scan root path.
	// Only applies when using recursive scans.
	FileListFlags_FileNameOnly       = (1 << 2),

	// Windows only: Retrieves the shell type string used to describe
	// a file type and used to sort by type in the explorer.
	FileListFlags_GetTypeStrings     = (1 << 3),
};

struct FileListInfo;

class FileList : public lang::Noncopyable
{
public:
	FileList();
	FileList(const String &directoryPath, FileListStyle listStyle = FileListStyle_All, uint32 listFlags = 0);
	~FileList();

	bool open(const String &directoryPath, FileListStyle listStyle = FileListStyle_All, uint32 listFlags = 0);
	void close();

	bool next(FileListEntry &entry);
	bool rewind();

	bool isDone() const;
	
	void setRegex(const String &pattern);

	std::vector<FileListEntry> getFullListing();

private:
	// Hidden implementation details
	FileListInfo *m_info = nullptr;

	bool m_done = false;

	String m_directoryPath;
	FileListStyle m_listStyle = FileListStyle_All;
	SizeType m_listFlags = 0;

	void *m_regex = nullptr;
};

TS_END_PACKAGE1()

