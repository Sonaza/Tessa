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
		filepath = std::move(other.filepath);
		rootDirectory = std::move(other.rootDirectory);
		std::swap(isDir, other.isDir);
	}
	return *this;
}

const String &FileEntry::getFilepath() const
{
	return filepath;
}

const String FileEntry::getFullFilepath() const
{
	return joinPaths(rootDirectory, filepath);
}

bool FileEntry::isDirectory() const
{
	return isDir;
}

bool FileEntry::isFile() const
{
	return !isDir;
}

TS_END_PACKAGE1()
