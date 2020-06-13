#include "Precompiled.h"

#if TS_PLATFORM == TS_LINUX

#include "ts/file/FileList.h"
#include "ts/file/FileUtils.h"

#if __has_include(<filesystem>)

	#include <filesystem>
	namespace fs = std::filesystem;

#elif __has_include(<experimental/filesystem>)

	#include <experimental/filesystem>
	namespace fs = std::experimental::filesystem;

#endif

// #include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>

TS_PACKAGE1(file)

struct FileListInfo
{
	fs::recursive_directory_iterator iter;
};

FileList::FileList()
{
}

FileList::FileList(const String &path, FileListStyle listStyle, uint32 listFlags)
{
	open(path, listStyle, listFlags);
}

FileList::~FileList()
{
	close();
}

bool FileList::open(const String &path, FileListStyle listStyle, uint32 listFlags)
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
	
	std::error_code ec;
	m_info->iter = fs::recursive_directory_iterator(path.toUtf8().c_str(), ec);
	if (ec)
	{
		TS_LOG_ERROR("Failed to initialize FileList iterator: %s  Path: %s", ec.message(), path);
		return false;
	}

	m_directoryPath = path;
	m_listStyle = listStyle;
	m_listFlags = listFlags;

	return true;
}

void FileList::close()
{
	if (m_info != nullptr)
	{
		delete m_info;
		m_info = nullptr;
	}

	if (m_regex != nullptr)
	{
		std::regex *regex = static_cast<std::regex *>(m_regex);
		delete regex;
		m_regex = nullptr;
	}
}

bool FileList::next(FileListEntry &output)
{
	TS_ASSERT(m_info != nullptr && "FileList is not opened.");
	
	if (m_done == true)
		return false;

	while (m_info->iter != fs::end(m_info->iter))
	{
		const fs::directory_entry &entry = *m_info->iter;
		
		if ((m_listStyle & priv::ListStyleBits_Recursive) == 0)
			m_info->iter.disable_recursion_pending();
		
		std::error_code ec;
		m_info->iter.increment(ec);
		if (ec)
		{
			TS_LOG_ERROR("Directory iterator increment failed.");
			return false;
		}
		
		String relativePath(entry.path().u32string());
		String filename(entry.path().filename().u32string());
		
		// std::cout << entry.path() << "\n";
		// std::cout << relativePath.toUtf8() << "\n";
		// TS_PRINTF("%s\n", relativePath.toUtf8());
		
		const bool isDir = fs::is_directory(entry.status());
		const bool isFile = fs::is_regular_file(entry.status());
		
		if (isDir == true)
		{
			if (filename == "." || filename == "..")
				continue;

			if ((m_listStyle & priv::ListStyleBits_Directories) == 0)
				continue;
		}
		else if (isFile == true)
		{
			if ((m_listStyle & priv::ListStyleBits_Files) == 0)
				continue;

			if (m_regex != nullptr)
			{
				std::regex &regex = *static_cast<std::regex *>(m_regex);
				if (!std::regex_search(filename.toUtf8(), regex))
				{
					TS_PRINTF("File %s does not match the regex.\n", filename);
					continue;
				}
			}
		}
		else
		{
			continue;
		}

		if ((m_listFlags & FileListFlags_FileNameOnly) != 0)
		{
			output.m_filename = std::move(filename);
		}
		else
		{
			String path = stripRootPath(relativePath, m_directoryPath);
			output.m_filename = std::move(path);
		}

		if ((m_listFlags & FileListFlags_ExcludeRootPath) == 0)
			output.m_rootpath = m_directoryPath;
		
		struct stat st;
		if (stat(relativePath.toUtf8().c_str(), &st) == 0)
		{
			output.m_lastModified = convertUnixTimeToWindowsFileTime(st.st_mtim);
			output.m_filesize = st.st_size;
		}

		output.m_directory = isDir;

		if ((m_listFlags & FileListFlags_GetTypeStrings) != 0)
		{
			output.m_typestring = getShellFileType(filename);
		}

		return true;
	}
	
	m_done = true;
	return false;
}

bool FileList::rewind()
{
	TS_ASSERT(m_info != nullptr && "FileList is not opened.");
	if (m_info == nullptr)
		return false;
	
	std::error_code ec;
	m_info->iter = fs::recursive_directory_iterator(m_directoryPath.toUtf8().c_str(), ec);
	if (ec)
	{
		TS_LOG_ERROR("Failed to reinitialize FileList iterator: %s  Path: %s", ec.message(), m_directoryPath);
		return false;
	}
	
	m_done = false;
	return true;
}

bool FileList::isDone() const
{
	return m_info != nullptr && m_done;
}

void FileList::setRegex(const String &pattern)
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

	m_regex = regex;
}

std::vector<FileListEntry> FileList::getFullListing()
{
	rewind();

	std::vector<FileListEntry> list;
	FileListEntry entry;
	while (next(entry))
		list.push_back(std::move(entry));

	return list;
}

TS_END_PACKAGE1()

#endif
