#include "Precompiled.h"
#include "ts/tessa/file/FileEntryW.h"

#include "ts/tessa/file/FileUtils.h"

TS_PACKAGE1(file)

FileEntryW::FileEntryW(FileEntryW &&other)
{
	*this = std::move(other);
}

FileEntryW &FileEntryW::operator=(FileEntryW &&other)
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

const std::wstring &FileEntryW::getFilepath() const
{
	return _filepath;
}

const std::wstring FileEntryW::getFullFilepath() const
{
	return utils::joinPaths(_rootDirectory, _filepath);
}

bool FileEntryW::isDirectory() const
{
	return _isDir;
}

bool FileEntryW::isFile() const
{
	return !_isDir;
}

TS_END_PACKAGE1()
