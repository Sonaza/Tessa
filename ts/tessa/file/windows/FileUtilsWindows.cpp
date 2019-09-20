#include "Precompiled.h"

#if TS_PLATFORM == TS_WINDOWS

#include "ts/tessa/file/FileUtils.h"
#include "ts/tessa/string/StringUtils.h"

#include <stack>
#include <cstdio>
#include <regex>

#include "ts/tessa/common/IncludeWindows.h"
#include "ts/tessa/common/WindowsUtils.h"

#include <shlwapi.h>
#include <io.h> 

#define access  _access_s
#define waccess _waccess_s

#define MAX_PATH_LENGTH MAX_PATH

TS_PACKAGE1(file)

extern bool isAbsolutePath(const String &path)
{
	return PathIsRelativeW(path.toWideString().c_str()) == FALSE;
}

extern String normalizePath(const String &path, string::Character delimiter)
{
	String workpath = path;
	string::trimWhitespace(workpath);

	const bool isUncPath = PathIsUNCW(path.toWideString().c_str()) == TRUE;

	std::vector<String> splitPath = string::splitString(workpath, TS_ALL_PATH_DELIMITERS);

	int32 ignore = 0;
	std::stack<String *> pathTokens;
	for (auto it = splitPath.rbegin(); it != splitPath.rend(); ++it)
	{
		String &p = *it;

		if (p.isEmpty() || p == ".")
			continue;

		if (p == "..")
		{
			ignore++;
			continue;
		}

		if (ignore > 0)
		{
			ignore--;
			continue;
		}

		pathTokens.push(&p);
	}

	String normalized;
	normalized.reserve(path.getSize() / 2);

	if (isUncPath)
		normalized.append("\\\\");

	while (!pathTokens.empty())
	{
		normalized.append(*pathTokens.top());

		if (pathTokens.size() > 1)
			normalized.append(delimiter);

		pathTokens.pop();
	}
	return normalized;
}

extern bool exists(const String &path)
{
	return PathFileExistsW(path.toWideString().c_str()) == TRUE;
}

extern bool isFile(const String &path)
{
	return PathIsDirectoryW(path.toWideString().c_str()) == FALSE;
}

extern bool isDirectory(const String &path)
{
	return !isFile(path);
}

extern bool removeFile(const String &path)
{
	if (DeleteFileW(path.toWideString().c_str()) == FALSE)
	{
		TS_LOG_ERROR("Unable to remove file or directory. File: %s. Error: %s", path, windows::getLastErrorAsString());
		return false;
	}
	return true;
}

extern FileTime getFileModifiedTime(const String &path)
{
	WIN32_FILE_ATTRIBUTE_DATA data;
	if (GetFileAttributesExW(path.toWideString().c_str(), GetFileExInfoStandard, &data))
	{
		if (data.nFileSizeLow == 0 && data.nFileSizeHigh == 0)
			return -1;

		FILETIME ft = data.ftLastWriteTime;
		LARGE_INTEGER li;
		li.LowPart = ft.dwLowDateTime;
		li.HighPart = ft.dwHighDateTime;
		return li.QuadPart;
	}
	return -1;
}

extern String getExecutableDirectory()
{
	// Cache executable path
	static String executablePath;
	if (!executablePath.isEmpty())
		return executablePath;

	wchar_t buffer[MAX_PATH_LENGTH] = { 0 };
	GetModuleFileNameW(nullptr, buffer, MAX_PATH_LENGTH);
	executablePath = getDirname(String(buffer));
	return executablePath;
}

extern String getWorkingDirectory()
{
	wchar_t buffer[MAX_PATH_LENGTH] = { 0 };
	GetCurrentDirectoryW(MAX_PATH_LENGTH, buffer);
	return String(buffer);
}

extern void setWorkingDirectory(const String &path)
{
	TS_ASSERT(path.getSize() < MAX_PATH_LENGTH);
	SetCurrentDirectoryW(path.toWideString().c_str());
}

TS_END_PACKAGE1()

#endif
