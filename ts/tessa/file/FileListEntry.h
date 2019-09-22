#pragma once

#include "ts/tessa/file/FileTime.h"

TS_PACKAGE1(file)

class FileListEntry
{
public:
	FileListEntry() = default;

	FileListEntry(FileListEntry &&other);
	FileListEntry &operator=(FileListEntry &&other);

	// Returns the filename (including relative path if 
	const String &getFilename() const;
	const String getFullPath() const;

	FileTime getLastModified() const;
	BigSizeType getSize() const;

	bool isDirectory() const;
	bool isFile() const;

private:
	friend class FileList;

	String m_filename;
	String m_rootpath;
	
	FileTime m_lastModified = 0;
	BigSizeType m_filesize = 0;
	bool m_directory = false;
};

TS_END_PACKAGE1()

