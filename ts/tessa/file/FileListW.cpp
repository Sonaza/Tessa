#include "Precompiled.h"
#include "ts/tessa/file/FileListW.h"

#include "ts/tessa/file/FileUtils.h"

#include "ext/dirent.h"

TS_PACKAGE1(file)

FileListW::FileListW()
{
}

FileListW::FileListW(const std::wstring &directoryPath, bool skipDotEntries, FileListStyle style)
{
	open(directoryPath, skipDotEntries, style);
}

FileListW::~FileListW()
{
	close();
}

bool FileListW::open(const std::wstring &directoryPath, bool skipDotEntries, FileListStyle style)
{
	MutexGuard lock(mutex);
	TS_ASSERT(_dirStack.empty() && "FileListW is already opened.");
	if (!_dirStack.empty())
		return false;

	WDIR *dir = wopendir(directoryPath.c_str());
	if (dir == nullptr)
	{
		TS_WLOG_ERROR("Failed to open directory for listing: '%s'\n", directoryPath);
		return false;
	}

	_directoryPath = directoryPath;
	_style = style;
	_skipDotEntries = skipDotEntries;

	_dirStack.push(DirectoryFrame(dir, directoryPath));

	return true;
}

void FileListW::close()
{
	MutexGuard lock(mutex);
	while (!_dirStack.empty())
	{
		wclosedir((WDIR*)_dirStack.top().ptr);
		_dirStack.pop();
	}
}

bool FileListW::next(FileEntryW &entry)
{
	MutexGuard lock(mutex);
	TS_ASSERT(!_dirStack.empty() && "FileListW is not opened.");
	if (_dirStack.empty() || _done == true)
		return false;

	wdirent *ent;
	while (true)
	{
		const size_t depth = _dirStack.size();

		DirectoryFrame &frame = _dirStack.top();
		ent = wreaddir((WDIR*)frame.ptr);
		if (ent != nullptr)
		{
			const wchar_t *filename = ent->d_name;
			std::wstring currentPath = depth > 1 ? (frame.rootPath + L"/" + filename) : filename;

			const bool isDir = (ent->d_type & DT_DIR) > 0;
			if (isDir == true)
			{
				if (wcscmp(filename, L".") == 0 || wcscmp(filename, L"..") == 0)
				{
					if (_skipDotEntries == true)
						continue;
				}
				else if ((_style & priv::ListStyleBits_Recursive) > 0)
				{
					WDIR *dirPtr = wopendir(currentPath.c_str());
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
						TS_WPRINTF("File %s does not match the glob.\n", filename);
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
			wclosedir((WDIR*)frame.ptr);
			_dirStack.pop();
			continue;
		}
		break;
	}
	_done = true;
	return false;
}

void FileListW::rewind()
{
	MutexGuard lock(mutex);
	TS_ASSERT(!_dirStack.empty() && "FileListW is not opened.");

	// Close all other directories but the bottom-most
	while (_dirStack.size() > 1)
	{
		wclosedir((WDIR*)_dirStack.top().ptr);
		_dirStack.pop();
	}
	wrewinddir((WDIR*)_dirStack.top().ptr);
	_done = false;
}

bool FileListW::isDone() const
{
	MutexGuard lock(mutex);
	return _done;
}

void FileListW::setGlobRegex(const std::wstring &pattern)
{
	MutexGuard lock(mutex);

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

std::vector<FileEntryW> FileListW::getFullListing()
{
	rewind();

	std::vector<FileEntryW> list;
	FileEntryW entry;
	while (next(entry))
		list.push_back(std::move(entry));

	return list;
}

TS_END_PACKAGE1()
