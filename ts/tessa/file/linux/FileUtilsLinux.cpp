#include "Precompiled.h"

#if TS_PLATFORM == TS_LINUX

#include "FileUtils.h"
#include "ts/tessa/string/StringUtils.h"

#include <stack>
#include <cstdio>

#include <unistd.h>
#define waccess access

#define MAX_PATH_LENGTH PATH_MAX

TS_PACKAGE1(file)

extern bool isAbsolutePath(const String &path)
{
	if (path.getSize() >= 1)
		return path[0] == '/';
	return false;
}

extern String normalizePath(const String &path, string::Character delimiter)
{
	String workpath = path;
	string::trimWhitespace(workpath);

	std::vector<String> splitPath = string::splitString(workpath, TS_ALL_PATH_DELIMITERS);

	// TODO: probably doesn't support /../asdfasd style paths

	int32 ignore = 0;
	std::stack<String*> pathTokens;
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
	// 0 flag tests for existence only
	return access(path.toUtf8().c_str(), 0) == 0;
}

extern bool isFile(const String &path)
{
	struct stat s;
	if (stat(path.toUtf8().c_str(), &s) == 0)
	{
		if (s.st_mode & S_IFREG)
			return true;
	}
	return false;
}

extern bool isDirectory(const String &path)
{
	return !isFile(path);
}

extern bool removeFile(const String &path)
{
	if (remove(path.toUtf8().c_str()) == 0)
		return true;
	TS_LOG_ERROR("Unable to remove file or directory. File: %s. Error: %s", path, strerror(errno));
	return false;
}

extern bool getFileModifiedTime(const String &path, int64 &modifiedTime)
{
	
	return false;
}

extern String getExecutableDirectory()
{
	// Cache executable path
	static String executablePath;
	if (!executablePath.isEmpty())
		return executablePath;

	char buffer[MAX_PATH_LENGTH] = { 0 };
	BigSizeType count = readlink("/proc/self/exe", buffer, MAX_PATH_LENGTH);
	if (count != -1)
		executablePath = getDirname(String(buffer));

	return executablePath;
}

extern String getWorkingDirectory()
{
	char buffer[MAX_PATH_LENGTH] = { 0 };
	getcwd(buffer, MAX_PATH_LENGTH);
	return String(buffer);
}

extern void setWorkingDirectory(const String &path)
{
	TS_ASSERT(path.getSize() < MAX_PATH_LENGTH);
	chdir(path.toUtf8().c_str());
}

extern FileTime getFileModifiedTime(const String &path)
{
	struct stat st;
	stat(path.toUtf8().c_str(), &st);
	// Converts from unix epoch time to 100-nanosecond units as in windows
	return getFileTimeFromTimestamp((uint64_t)st.st_mtim.tv_sec * 1000 + (uint64_t)st.st_mtim.tv_nsec / 1000000);
}

TS_END_PACKAGE1()

#endif