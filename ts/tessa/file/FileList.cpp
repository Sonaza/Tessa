#include "Precompiled.h"
#include "ts/tessa/file/FileList.h"

#include "ts/tessa/file/FileUtils.h"

#include "ext/dirent.h"

TS_PACKAGE1(file)

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
			const char *filename = ent->d_name;
			std::string currentPath = depth > 1 ? (frame.rootPath + "/" + filename) : filename;

			const bool isDir = (ent->d_type & DT_DIR) > 0;
			if (isDir == true)
			{
				if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0)
				{
					if (_skipDotEntries == true)
						continue;
				}
				else if ((_style & priv::ListStyleBits_Recursive) > 0)
				{
					DIR *dirPtr = opendir(currentPath.c_str());
					if (dirPtr != nullptr)
						_dirStack.push(DirectoryFrame(dirPtr, currentPath));
				}

				if ((_style & priv::ListStyleBits_Directories) == 0)
					continue;
			}
			else
			{
				if ((_style & priv::ListStyleBits_Files) == 0)
					continue;

				if (_glob != nullptr)
				{
					if (!std::regex_search(filename, *_glob))
					{
						TS_PRINTF("File %s does not match the glob.\n", filename);
						continue;
					}
				}
			}

			entry._filepath = std::move(currentPath);
			entry._rootDirectory = frame.rootPath;
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

bool FileList::isDone() const
{
	std::lock_guard<std::mutex> mg(mutex);
	return _done;
}

void FileList::setGlobRegex(const std::string &pattern)
{
	std::lock_guard<std::mutex> mg(mutex);

	GlobRegexType *regex = nullptr;
	try
	{
		regex = new GlobRegexType(pattern, std::regex_constants::ECMAScript);
	}
	catch (std::regex_error &e)
	{
		TS_UNUSED_VARIABLE(e);
		TS_ASSERTF(false, "Given regex pattern is invalid. Error: %s\n", e.what());
		return;
	}

	_glob.reset(regex);
}

std::vector<FileEntry> FileList::getFullListing()
{
	rewind();

	std::vector<FileEntry> list;
	FileEntry entry;
	while (next(entry))
		list.push_back(std::move(entry));

	return list;
}

TS_END_PACKAGE1()
