#include "Precompiled.h"

#if TS_PLATFORM == TS_WINDOWS

#include "ts/file/FileList.h"
#include "ts/file/FileUtils.h"

#include "ts/lang/common/IncludeWindows.h"
#include <shellapi.h>

TS_PACKAGE1(file)

struct FileListInfo
{
	struct DirectoryFrame
	{
		HANDLE handle;
		String absolutePath;
	};
	std::stack<DirectoryFrame> directoryStack;
};

FileList::FileList()
{
}

FileList::FileList(const String &path, FileListStyle listStyle, uint32_t listFlags)
{
	open(path, listStyle, listFlags);
}

FileList::~FileList()
{
	close();
}

bool FileList::open(const String &path, FileListStyle listStyle, uint32_t listFlags)
{
	TS_ASSERT(!path.isEmpty());
	
	TS_ASSERT(m_info == nullptr && "FileList is already opened.");
	if (m_info != nullptr)
		return false;
	
	m_info = new(std::nothrow) FileListInfo;
	if (m_info == nullptr)
	{
		TS_ASSERT(!"Failed to init file list info.");
		return false;
	}

	if (!m_info->directoryStack.empty())
		return false;

	if (!exists(path) || !isDirectory(path))
	{
		TS_WLOG_ERROR("Path does not exist or isn't a directory. Path: %s", path);
		return false;
	}

	m_directoryPath = path;
	m_listStyle = listStyle;
	m_listFlags = listFlags;

	// Entries are pushed with null handles since "opening"
	// the handle already also queries for the first file.
	m_info->directoryStack.push(FileListInfo::DirectoryFrame{ nullptr, m_directoryPath });

	return true;
}

void FileList::close()
{
	if (m_info != nullptr)
	{
		while (!m_info->directoryStack.empty())
		{
			FindClose(m_info->directoryStack.top().handle);
			m_info->directoryStack.pop();
		}
	
		delete m_info;
		m_info = nullptr;
	}
	
	if (m_regex != nullptr)
	{
		std::wregex *regex = static_cast<std::wregex *>(m_regex);
		delete regex;
		m_regex = nullptr;
	}
}

bool FileList::next(FileListEntry &entry)
{
	TS_ASSERT(m_info != nullptr && "FileList is not opened.");
	TS_ASSERT(!m_info->directoryStack.empty() && "FileList is not opened.");
	
	if (m_info == nullptr)
		return false;
	
	if (m_info->directoryStack.empty() || m_done == true)
		return false;

	while (true)
	{
		const size_t depth = m_info->directoryStack.size();

		FileListInfo::DirectoryFrame &frame = m_info->directoryStack.top();

		WIN32_FIND_DATAW findData = {};

		bool entryFound = false;

		if (frame.handle == nullptr)
		{
			String searchPath = file::joinPaths(frame.absolutePath, L"*");

			DWORD flags = 0;
			if ((m_listFlags & FileListFlags_LargeFetch) != 0)
				flags |= FIND_FIRST_EX_LARGE_FETCH;

			HANDLE handle = FindFirstFileExW(
				searchPath.toWideString().c_str(),
				FindExInfoBasic, // cAlternateFileName is always null with this flag
				&findData,
				FindExSearchNameMatch,
				nullptr,
				flags
			);
			if (handle == INVALID_HANDLE_VALUE)
			{
				TS_WLOG_ERROR("Failed to start file listing in a directory. Path: %s. Error: %s",
					frame.absolutePath,
					windows::getLastErrorAsString()
				);
			}
			else
			{
				frame.handle = handle;
				entryFound = true;
			}
		}
		else
		{
			if (FindNextFileW(frame.handle, &findData) == TRUE)
			{
				entryFound = true;
			}
			else if (GetLastError() != ERROR_NO_MORE_FILES)
			{
				TS_WLOG_ERROR("FindNextFileW failed. Error: %s",
					windows::getLastErrorAsString()
				);
			}
		}

		if (entryFound)
		{
			String filename(findData.cFileName);

			const bool isDir = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
			if (isDir == true)
			{
				if (filename == "." || filename == "..")
				{
					continue;
				}
				else if ((m_listStyle & priv::ListStyleBits_Recursive) != 0)
				{
					String absolutePath = joinPaths(frame.absolutePath, filename);

					// Sanity check assert
					TS_ASSERT(exists(absolutePath) && isDirectory(absolutePath));
					m_info->directoryStack.push(
						FileListInfo::DirectoryFrame{ nullptr, absolutePath }
					);
				}

				if ((m_listStyle & priv::ListStyleBits_Directories) == 0)
					continue;
			}
			else
			{
				if ((m_listStyle & priv::ListStyleBits_Files) == 0)
					continue;

				if (m_regex != nullptr)
				{
					std::wregex &regex = *static_cast<std::wregex *>(m_regex);
					if (!std::regex_search(filename.toWideString(), regex))
					{
						TS_PRINTF("File %s does not match the regex.\n", filename);
						continue;
					}
				}
			}
			
			if ((m_listFlags & FileListFlags_FileNameOnly) != 0 || depth == 1)
			{
				entry.m_filename = std::move(filename);
			}
			else
			{
				String path = stripRootPath(frame.absolutePath, m_directoryPath);
				appendPath(path, filename);
				entry.m_filename = std::move(path);
			}

			if ((m_listFlags & FileListFlags_ExcludeRootPath) == 0)
				entry.m_rootpath = frame.absolutePath;

			entry.m_lastModified = windows::convertLargeIntegerTo64bit(
				findData.ftLastWriteTime.dwLowDateTime,
				findData.ftLastWriteTime.dwHighDateTime);

			entry.m_filesize = windows::convertLargeIntegerTo64bit(
				findData.nFileSizeLow, findData.nFileSizeHigh);

			entry.m_directory = isDir;

			if ((m_listFlags & FileListFlags_GetTypeStrings) != 0)
			{
				entry.m_typestring = getShellFileType(findData.cFileName);
			}

			return true;
		}

		if (depth > 1)
		{
			FindClose(frame.handle);
			m_info->directoryStack.pop();
			continue;
		}

		break;
	}
	m_done = true;
	return false;
}

bool FileList::rewind()
{
	TS_ASSERT(m_info != nullptr && "FileList is not opened.");
	TS_ASSERT(!m_info->directoryStack.empty() && "FileList is not opened.");
	
	if (m_info == nullptr)
		return false;
	
	// Close and pop all other directories but the bottom-most
	while (m_info->directoryStack.size() > 1)
	{
		FindClose(m_info->directoryStack.top().handle);
		m_info->directoryStack.pop();
	}
	
	// Close the last one and reset handle to null
	FileListInfo::DirectoryFrame &top = m_info->directoryStack.top();
	if (top.handle != nullptr)
		FindClose(top.handle);

	top.handle = nullptr;

	m_done = false;
	return true;
}

bool FileList::isDone() const
{
	return m_info != nullptr && m_done;
}

void FileList::setRegex(const String &pattern)
{
	std::wregex *regex = nullptr;
	try
	{
		regex = new std::wregex(pattern.toWideString(), std::regex_constants::ECMAScript);
	}
	catch (std::regex_error &e)
	{
		TS_UNUSED_VARIABLE(e);
		TS_ASSERTF(false, "Given regex pattern is invalid. Error: %s\n", e.what());
		return;
	}

	m_regex = regex;
}

std::vector<FileListEntry> FileList::getFullListing()
{
	TS_ASSERT(m_info != nullptr && "FileList is not opened.");
	
	rewind();

	std::vector<FileListEntry> list;
	FileListEntry entry;
	while (next(entry))
		list.push_back(std::move(entry));

	return list;
}

TS_END_PACKAGE1()

#endif
