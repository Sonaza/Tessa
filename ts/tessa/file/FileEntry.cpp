#include "Precompiled.h"
#include "ts/tessa/file/FileEntry.h"

#include "ts/tessa/file/FileUtils.h"

TS_PACKAGE1(file)

FileEntry::FileEntry(FileEntry &&other)
{
	*this = std::move(other);
}

FileEntry &FileEntry::operator=(FileEntry &&other)
{
	if (this != &other)
	{
		m_basename = std::move(other.m_basename);
		m_rootpath = std::move(other.m_rootpath);

		m_lastModified = std::exchange(other.m_lastModified, 0);
		m_filesize = std::exchange(other.m_filesize, 0);
		m_directory = std::exchange(other.m_directory, false);
	}
	return *this;
}

const String &FileEntry::getBasename() const
{
	return m_basename;
}

const String FileEntry::getFullPath() const
{
	return joinPaths(m_rootpath, m_basename);
}

FileTime FileEntry::getLastModified() const
{
	return m_lastModified;
}

BigSizeType FileEntry::getSize() const
{
	return m_filesize;
}

bool FileEntry::isDirectory() const
{
	return m_directory;
}

bool FileEntry::isFile() const
{
	return !m_directory;
}

TS_END_PACKAGE1()
