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
		_filepath = std::move(other._filepath);
		_rootDirectory = std::move(other._rootDirectory);
		_isDir = other._isDir;
		other._isDir = false;
	}
	return *this;
}

const std::string &FileEntry::getFilepath() const
{
	return _filepath;
}

const std::string FileEntry::getFullFilepath() const
{
	return utils::joinPaths(_rootDirectory, _filepath);
}

bool FileEntry::isDirectory() const
{
	return _isDir;
}

bool FileEntry::isFile() const
{
	return !_isDir;
}

TS_END_PACKAGE1()
