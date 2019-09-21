#pragma once

#include "ts/tessa/file/FileTime.h"

TS_PACKAGE1(file)

class FileEntry
{
public:
	FileEntry() = default;

	FileEntry(FileEntry &&other);
	FileEntry &operator=(FileEntry &&other);

	const String &getBasename() const;
	const String getFullPath() const;

	FileTime getLastModified() const;
	BigSizeType getSize() const;

	bool isDirectory() const;
	bool isFile() const;

private:
	friend class FileList;

	String m_basename;
	String m_rootpath;
	
	FileTime m_lastModified = 0;
	BigSizeType m_filesize = 0;
	bool m_directory = false;
};

TS_END_PACKAGE1()

