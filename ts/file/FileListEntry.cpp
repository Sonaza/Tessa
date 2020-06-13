#include "Precompiled.h"
#include "ts/file/FileListEntry.h"

#include "ts/file/FileUtils.h"

TS_PACKAGE1(file)

FileListEntry::FileListEntry(FileListEntry &&other)
{
	*this = std::move(other);
}

FileListEntry &FileListEntry::operator=(FileListEntry &&other)
{
	if (this != &other)
	{
		m_filename = std::move(other.m_filename);
		m_rootpath = std::move(other.m_rootpath);

		m_lastModified = std::exchange(other.m_lastModified, 0);
		m_filesize = std::exchange(other.m_filesize, 0);
		m_directory = std::exchange(other.m_directory, false);
	}
	return *this;
}

const String &FileListEntry::getFilename() const
{
	return m_filename;
}

const String FileListEntry::getFullPath() const
{
	return joinPaths(m_rootpath, m_filename);
}

const String &FileListEntry::getTypestring() const
{
	return m_typestring;
}

FileTime FileListEntry::getLastModified() const
{
	return m_lastModified;
}

BigSizeType FileListEntry::getSize() const
{
	return m_filesize;
}

bool FileListEntry::isDirectory() const
{
	return m_directory;
}

bool FileListEntry::isFile() const
{
	return !m_directory;
}

TS_END_PACKAGE1()
