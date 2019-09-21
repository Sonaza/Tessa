#include "Precompiled.h"

#if TS_PLATFORM == TS_LINUX

#include "ts/tessa/file/FileList.h"
#include "ts/tessa/file/FileUtils.h"

#include <dirent.h>

TS_PACKAGE1(file)

FileList::FileList()
{
}

FileList::FileList(const String &path, FileListStyle listStyle, SizeType listFlags)
{
	open(path, listStyle, listFlags);
}

FileList::~FileList()
{
	close();
}

bool FileList::open(const String &path, FileListStyle listStyle, SizeType listFlags)
{
	TS_ASSERT(!path.isEmpty());

	TS_ASSERT(m_directoryStack.empty() && "FileList is already opened.");
	if (!m_directoryStack.empty())
		return false;

	DIR *dir = opendir(path.toUtf8().c_str());
	if (dir == nullptr)
		return false;

	m_directoryPath = path;
	m_listStyle = listStyle;
	m_listFlags = listFlags;

	m_directoryStack.push(DirectoryFrame(dir, m_directoryPath, ""));

	return true;
}

void FileList::close()
{
	while (!m_directoryStack.empty())
	{
		readdir((DIR*)m_directoryStack.top().handle);
		m_directoryStack.pop();
	}

	if (m_globRegex != nullptr)
	{
		std::regex *regex = static_cast<std::regex *>(m_globRegex);
		delete regex;
		m_globRegex = nullptr;
	}
}

bool FileList::next(FileEntry &entry)
{
	TS_ASSERT(!m_directoryStack.empty() && "FileList is not opened.");
	if (m_directoryStack.empty() || m_done == true)
		return false;

	dirent *ent;
	while (true)
	{
		const size_t depth = m_directoryStack.size();

		DirectoryFrame &frame = m_directoryStack.top();
		ent = readdir((DIR*)frame.handle);
		if (ent != nullptr)
		{
			String filename(ent->d_name);

			String relativePath = joinPaths(frame.relativePath, filename);

			const bool isDir = (ent->d_type & DT_DIR) > 0;
			if (isDir == true)
			{
				if (filename == "." || filename == "..")
				{
					if ((m_listFlags & FileListFlags_SkipDotEntries) != 0)
						continue;
				}
				else if ((m_listStyle & priv::ListStyleBits_Recursive) > 0)
				{
					String absolutePath = joinPaths(frame.absolutePath, relativePath);

					DIR *dirPtr = opendir(absolutePath.toUtf8().c_str());
					TS_ASSERT(dirPtr != nullptr);
					if (dirPtr != nullptr)
						m_directoryStack.push(DirectoryFrame(dirPtr, frame.absolutePath, relativePath));
				}

				if ((m_listStyle & priv::ListStyleBits_Directories) == 0)
					continue;
			}
			else
			{
				if ((m_listStyle & priv::ListStyleBits_Files) == 0)
					continue;

				if (m_globRegex != nullptr)
				{
					std::regex &regex = *static_cast<std::regex *>(m_globRegex);
					if (!std::regex_search(filename.toUtf8(), regex))
					{
						TS_PRINTF("File %s does not match the glob.\n", filename);
						continue;
					}
				}
			}

			entry.m_basename = std::move(relativePath);
			entry.m_rootpath = frame.absolutePath;
			
			struct stat st;
			if (stat(entry.getFullPath().toUtf8().c_str(), &st) == 0)
			{
				entry.m_lastModified = convertUnixTimeToWindowsFileTime(st.st_mtim);
				entry.m_filesize = st.st_size;
			}

			entry.m_directory = isDir;
			return true;
		}
		if (depth > 1)
		{
			readdir((DIR*)frame.handle);
			m_directoryStack.pop();
			continue;
		}
		break;
	}
	m_done = true;
	return false;
}

void FileList::rewind()
{
	TS_ASSERT(!m_directoryStack.empty() && "FileList is not opened.");

	// Close all other directories but the bottom-most
	while (m_directoryStack.size() > 1)
	{
		readdir((DIR*)m_directoryStack.top().handle);
		m_directoryStack.pop();
	}
	rewinddir((DIR*)m_directoryStack.top().handle);
	m_done = false;
}

bool FileList::isDone() const
{
	return m_done;
}

void FileList::setGlobRegex(const String &pattern)
{
	std::regex *regex = nullptr;
	try
	{
		regex = new std::regex(pattern.toUtf8(), std::regex_constants::ECMAScript);
	}
	catch (std::regex_error &e)
	{
		TS_UNUSED_VARIABLE(e);
		TS_ASSERTF(false, "Given regex pattern is invalid. Error: %s\n", e.what());
		return;
	}

	m_globRegex = regex;
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

#endif
