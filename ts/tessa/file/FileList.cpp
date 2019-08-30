#include "Precompiled.h"
#include "ts/tessa/file/FileList.h"

#include "ts/tessa/file/FileUtils.h"

#if TS_PLATFORM == TS_WINDOWS

#include "ext/dirent.h"

#define _dirent    wdirent
#define _DIR       WDIR
#define _opendir   wopendir
#define _readdir   wreaddir
#define _closedir  wclosedir
#define _rewinddir wrewinddir
#define _toString  toWideString

typedef std::wregex GlobRegexType;

#else

#define _dirent    dirent
#define _opendir   opendir
#define _readdir   readdir
#define _closedir  closedir
#define _rewinddir rewinddir
#define _toString  toUtf8

typedef std::regex GlobRegexType;

#endif

TS_PACKAGE1(file)

FileList::FileList()
{
}

FileList::FileList(const String &pathParam, bool skipDotEntriesParam, FileListStyle styleParam)
{
	open(pathParam, skipDotEntries, style);
}

FileList::~FileList()
{
	close();
}

bool FileList::open(const String &pathParam, bool skipDotEntriesParam, FileListStyle styleParam)
{
	MutexGuard lock(mutex);

	TS_ASSERT(!pathParam.isEmpty());

	TS_ASSERT(directoryStack.empty() && "FileList is already opened.");
	if (!directoryStack.empty())
		return false;

	_DIR *dir = _opendir(pathParam._toString().c_str());
	if (dir == nullptr)
		return false;

	directoryPath = pathParam;
	style = styleParam;
	skipDotEntries = skipDotEntriesParam;

	directoryStack.push(DirectoryFrame(dir, pathParam));

	return true;
}

void FileList::close()
{
	MutexGuard lock(mutex);
	while (!directoryStack.empty())
	{
		_closedir((_DIR*)directoryStack.top().ptr);
		directoryStack.pop();
	}

	if (globRegex != nullptr)
	{
		GlobRegexType *regex = static_cast<GlobRegexType *>(globRegex);
		delete regex;
		globRegex = nullptr;
	}
}

bool FileList::next(FileEntry &entry)
{
	MutexGuard lock(mutex);

	TS_ASSERT(!directoryStack.empty() && "FileList is not opened.");
	if (directoryStack.empty() || done == true)
		return false;

	_dirent *ent;
	while (true)
	{
		const size_t depth = directoryStack.size();

		DirectoryFrame &frame = directoryStack.top();
		ent = _readdir((_DIR*)frame.ptr);
		if (ent != nullptr)
		{
			String filename(ent->d_name);

			String currentPath = depth > 1 ? (frame.rootPath + "/" + filename) : filename;

			const bool isDir = (ent->d_type & DT_DIR) > 0;
			if (isDir == true)
			{
				if (filename == "." || filename == "..")
				{
					if (skipDotEntries == true)
						continue;
				}
				else if ((style & priv::ListStyleBits_Recursive) > 0)
				{
					_DIR *dirPtr = _opendir(currentPath._toString().c_str());
					if (dirPtr != nullptr)
						directoryStack.push(DirectoryFrame(dirPtr, currentPath));
				}

				if ((style & priv::ListStyleBits_Directories) == 0)
					continue;
			}
			else
			{
				if ((style & priv::ListStyleBits_Files) == 0)
					continue;

				if (globRegex != nullptr)
				{
					GlobRegexType &regex = *static_cast<GlobRegexType *>(globRegex);
					if (!std::regex_search(filename._toString(), regex))
					{
						TS_PRINTF("File %s does not match the glob.\n", filename);
						continue;
					}
				}
			}

			entry.filepath = std::move(currentPath);
			entry.rootDirectory = frame.rootPath;
			entry.isDir = isDir;
			return true;
		}
		if (depth > 1)
		{
			_closedir((_DIR*)frame.ptr);
			directoryStack.pop();
			continue;
		}
		break;
	}
	done = true;
	return false;
}

void FileList::rewind()
{
	MutexGuard lock(mutex);
	TS_ASSERT(!directoryStack.empty() && "FileList is not opened.");

	// Close all other directories but the bottom-most
	while (directoryStack.size() > 1)
	{
		_closedir((_DIR*)directoryStack.top().ptr);
		directoryStack.pop();
	}
	_rewinddir((_DIR*)directoryStack.top().ptr);
	done = false;
}

bool FileList::isDone() const
{
	MutexGuard lock(mutex);
	return done;
}

void FileList::setGlobRegex(const String &pattern)
{
	MutexGuard lock(mutex);

	GlobRegexType *regex = nullptr;
	try
	{
		regex = new GlobRegexType(pattern._toString(), std::regex_constants::ECMAScript);
	}
	catch (std::regex_error &e)
	{
		TS_UNUSED_VARIABLE(e);
		TS_ASSERTF(false, "Given regex pattern is invalid. Error: %s\n", e.what());
		return;
	}

	globRegex = regex;
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
