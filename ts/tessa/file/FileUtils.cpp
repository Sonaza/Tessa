#include "Precompiled.h"
#include "ts/tessa/file/FileUtils.h"
#include "ts/tessa/string/StringUtils.h"

#include <stack>
#include <cstdio>
#include <regex>

#if TS_PLATFORM == TS_WINDOWS
	#include "ts/tessa/common/IncludeWindows.h"
	#include "ts/tessa/common/WindowsUtils.h"

	#include <shlwapi.h>
	#include <io.h> 

	#define access  _access_s
	#define waccess _waccess_s

	#define MAX_PATH_LENGTH MAX_PATH
#else
	#include <unistd.h>
	#define waccess access

	#define MAX_PATH_LENGTH PATH_MAX
#endif

TS_PACKAGE1(file)

namespace
{

bool isSlash(int32 c)
{
	return c == '/' || c == '\\';
}

}

extern bool isAbsolutePath(const String &path)
{

#if TS_PLATFORM == TS_WINDOWS
	if (path.getSize() >= 2)
	{
		// Match drive letter and colon pattern (e.g. C:)
		// Network drive paths start with two backslashes and are considered absolute as well.
		static std::wregex pathRegex(L"^([A-Za-z]:|\\\\)", std::regex_constants::ECMAScript);
		if (std::regex_search(path.toWideString(), pathRegex))
			return true;
	}
#else
	if (path.getSize() >= 1)
		return path[0] == '/';
#endif
	return false;
}

extern bool hasTrailingSlash(const String &path)
{
	return isSlash(path.back());
}

extern void removeTrailingSlashes(String &path)
{
	path.erase(std::find_if(path.rbegin(), path.rend(), [](int32 c)
	{
		return !isSlash(c);
	}).base(), path.end());
}

extern String removeTrailingSlashesCopy(const String &path)
{
	String out = path;
	removeTrailingSlashes(out);
	return out;
}

extern String joinPaths(const String &left, const String &right, string::Character delimiter)
{
	if (left.isEmpty() && right.isEmpty())
		return left;

	else if (left.isEmpty())
		return right;

	else if (right.isEmpty())
		return left;

	String result;
	result.reserve(left.getSize() + right.getSize() + 1);
	result.append(left);
	removeTrailingSlashes(result);
	result.append(delimiter + right);
	return result;
}

extern String normalizePath(const String &path, string::Character delimiter)
{
	String workpath = path;
	string::trimWhitespace(workpath);

	const bool isNetworkSharePath = (workpath.getSize() >= 2 && workpath[0] == '\\' && workpath[1] == '\\');

	std::vector<String> splitPath = string::splitString(workpath, TS_ALL_PATH_DELIMITERS);

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

	if (isNetworkSharePath)
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

extern String getDirname(const String &path, const String &delimiters)
{
	BigSizeType pos = path.findLastOf(delimiters, String::InvalidPos);
	if (pos != String::InvalidPos)
		return path.substring(0, pos);
	return String();
}

extern String getBasename(const String &path, bool stripExtension, const String &delimiters)
{
	BigSizeType extPos = stripExtension ? path.findLastOf('.', String::InvalidPos) : String::InvalidPos;

	BigSizeType delimPos = path.findLastOf(delimiters, String::InvalidPos);
	if (delimPos != String::InvalidPos)
		return path.substring(delimPos + 1, extPos - 1 - delimPos);

	return path.substring(0, extPos);
}

extern String getExtension(const String &path)
{
	// Finds the last delimiter or period
	BigSizeType pos = path.findLastOf(TS_ALL_PATH_DELIMITERS ".");
	if (pos != String::InvalidPos && path[pos] == '.')
		return path.substring(pos + 1);
	return String();
}

extern bool exists(const String &path)
{
#if TS_PLATFORM == TS_WINDOWS
	return PathFileExistsW(path.toWideString().c_str()) == TRUE;
#else
	// 0 flag tests for existence only
	return access(path.toUtf8().c_str(), 0) == 0;
#endif
}

extern bool isFile(const String &path)
{
#if TS_PLATFORM == TS_WINDOWS
	return PathIsDirectoryW(path.toWideString().c_str()) == FALSE;
#else
	struct stat s;
	if (stat(path.toUtf8().c_str(), &s) == 0)
	{
		if (s.st_mode & S_IFREG)
			return true;
	}
	return false;
#endif
}

extern bool isDirectory(const String &path)
{
	return !isFile(path);
}

extern bool removeFile(const String &path)
{
#if TS_PLATFORM == TS_WINDOWS
	if (DeleteFileW(path.toWideString().c_str()) != 0) // Non-zero marks success
		return true;

	TS_LOG_ERROR("Unable to remove file or directory. File: %s. Error: %s", path, windows::getLastErrorAsString());
#else
	if (remove(path.c_str()) == 0)
		return true;
	TS_LOG_ERROR("Unable to remove file or directory. File: %s. Error: %s", path, strerror(errno));
#endif
	return false;
}

extern String getExecutableDirectory()
{
	// Cache executable path
	static String executablePath;
	if (!executablePath.isEmpty())
		return executablePath;

#if TS_PLATFORM == TS_WINDOWS
	wchar_t buffer[MAX_PATH_LENGTH] = { 0 };
	GetModuleFileNameW(nullptr, buffer, MAX_PATH_LENGTH);
	executablePath = getDirname(String(buffer));
#else
	char buffer[MAX_PATH_LENGTH] = { 0 };
	BigSizeType count = readlink("/proc/self/exe", buffer, MAX_PATH_LENGTH);
	if (count != -1)
		executablePath = getDirname(String(buffer));
#endif
	return executablePath;
}

extern String getWorkingDirectory()
{
#if TS_PLATFORM == TS_WINDOWS
	wchar_t buffer[MAX_PATH_LENGTH] = { 0 };
	GetCurrentDirectoryW(MAX_PATH_LENGTH, buffer);
#else
	char buffer[MAX_PATH_LENGTH] = { 0 };
	getcwd(buffer, MAX_PATH_LENGTH);
#endif
	return String(buffer);
}

extern void setWorkingDirectory(const String &path)
{
	TS_ASSERT(path.getSize() < MAX_PATH_LENGTH);
#if TS_PLATFORM == TS_WINDOWS
	SetCurrentDirectoryW(path.toWideString().c_str());
#else
	chdir(path.toUtf8().c_str());
#endif
}

TS_END_PACKAGE1()

