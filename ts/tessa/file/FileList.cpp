#include "Precompiled.h"
#include "ts/tessa/file/FileList.h"

#include "ext/dirent.h"

TS_PACKAGE1(file)

const std::string &FileEntry::getFilepath() const
{
	return _filepath;
}

FileEntry::FileEntry(FileEntry &&other)
{
	*this = std::move(other);
}

FileEntry &FileEntry::operator=(FileEntry &&other)
{
	if (this != &other)
	{
		_filepath = std::move(other._filepath);
		_isDir = other._isDir;
		other._isDir = false;
	}
	return *this;
}

bool FileEntry::isDirectory() const
{
	return _isDir;
}

FileList::FileList()
{
}

FileList::FileList(const std::string &directoryPath, bool skipDotEntries, FileListStyle style)
{
	open(directoryPath, skipDotEntries, style);
}

FileList::~FileList()
{
	close();
}

bool FileList::open(const std::string &directoryPath, bool skipDotEntries, FileListStyle style)
{
	std::lock_guard<std::mutex> mg(mutex);
	TS_ASSERT(_dirStack.empty() && "FileList is already opened.");
	if (!_dirStack.empty())
		return false;

	DIR *dir = opendir(directoryPath.c_str());
	if (dir == nullptr)
		return false;

	_directoryPath = directoryPath;
	_style = style;
	_skipDotEntries = skipDotEntries;

	_dirStack.push(DirectoryFrame(dir, directoryPath));

	return true;
}

void FileList::close()
{
	std::lock_guard<std::mutex> mg(mutex);
	while (!_dirStack.empty())
	{
		closedir((DIR*)_dirStack.top().ptr);
		_dirStack.pop();
	}
}

bool FileList::next(FileEntry &entry)
{
	std::lock_guard<std::mutex> mg(mutex);
	TS_ASSERT(!_dirStack.empty() && "FileList is not opened.");
	if (_dirStack.empty() || _done == true)
		return false;

	dirent *ent;
	while (true)
	{
		const size_t depth = _dirStack.size();

		DirectoryFrame &frame = _dirStack.top();
		ent = readdir((DIR*)frame.ptr);
		if (ent != nullptr)
		{
			std::string currentPath = depth > 1 ? (frame.rootPath + "/" + ent->d_name) : ent->d_name;

			const bool isDir = (ent->d_type & DT_DIR) > 0;
			if (isDir == true)
			{
				if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
				{
					if (_skipDotEntries == true)
						continue;
				}
				else if ((_style & TS_FILELIST_STYLE_RECURSIVE) > 0)
				{
					DIR *dirPtr = opendir(currentPath.c_str());
					if (dirPtr != nullptr)
						_dirStack.push(DirectoryFrame(dirPtr, currentPath));
				}

				if ((_style & TS_FILELIST_STYLE_DIRECTORIES) == 0)
					continue;
			}
			else
			{
				if ((_style & TS_FILELIST_STYLE_FILES) == 0)
					continue;
			}

			entry._filepath = std::move(currentPath);
			entry._isDir = isDir;
			return true;
		}
		if (depth > 1)
		{
			closedir((DIR*)frame.ptr);
			_dirStack.pop();
			continue;
		}
		break;
	}
	_done = true;
	return false;
}

bool FileList::done() const
{
	std::lock_guard<std::mutex> mg(mutex);
	return _done;
}

void FileList::rewind()
{
	std::lock_guard<std::mutex> mg(mutex);
	TS_ASSERT(!_dirStack.empty() && "FileList is not opened.");

	// Close all other directories but the bottom-most
	while (_dirStack.size() > 1)
	{
		closedir((DIR*)_dirStack.top().ptr);
		_dirStack.pop();
	}
	rewinddir((DIR*)_dirStack.top().ptr);
	_done = false;
}

std::vector<FileEntry> FileList::getFullListing()
{
	rewind();

	std::vector<FileEntry> list;
	FileEntry entry;
	while (next(entry))
	{
		list.push_back(std::move(entry));
	}
	return list;
}

TS_END_PACKAGE1()
