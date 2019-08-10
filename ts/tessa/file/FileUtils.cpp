#include "Precompiled.h"
#include "ts/tessa/file/FileUtils.h"
#include "ts/tessa/lang/StringUtils.h"

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

TS_PACKAGE2(file, utils)

namespace
{

bool isSlash(Int32 c)
{
	return c == '/' || c == '\\' || c == L'/' || c == L'\\';
}

}

extern bool isAbsolutePath(const std::string &path)
{
#if TS_PLATFORM == TS_WINDOWS
	if (path.size() >= 2)
	{
		// Match drive letter and colon pattern (e.g. C:)
		static std::regex pathRegex("^([A-Za-z]:|\\\\)", std::regex_constants::ECMAScript);
		if (std::regex_search(path, pathRegex))
			return true;
	}
#else
	if (path.size() >= 1)
		return path[0] == '/';
#endif
	return false;
}

extern bool isAbsolutePath(const std::wstring &path)
{

#if TS_PLATFORM == TS_WINDOWS
	if (path.size() >= 2)
	{
		// Match drive letter and colon pattern (e.g. C:)
		static std::wregex pathRegex(L"^([A-Za-z]:|\\\\)", std::regex_constants::ECMAScript);
		if (std::regex_search(path, pathRegex))
			return true;
	}
#else
	TS_ASSERT(!"Wide variant not implemented on this platform.");
#endif
	return false;
}

extern bool hasTrailingSlash(const std::string &path)
{
	return isSlash(path.back());
}

extern bool hasTrailingSlash(const std::wstring &path)
{
	return isSlash(path.back());
}

extern void removeTrailingSlashes(std::string &path)
{
	path.erase(std::find_if(path.rbegin(), path.rend(), [](Int32 c)
	{
		return !isSlash(c);
	}).base(), path.end());
}

extern std::string removeTrailingSlashesCopy(const std::string &path)
{
	std::string out = path;
	removeTrailingSlashes(out);
	return out;
}

extern void removeTrailingSlashes(std::wstring &path)
{
	path.erase(std::find_if(path.rbegin(), path.rend(), [](Int32 c)
	{
		return !isSlash(c);
	}).base(), path.end());
}

extern std::wstring removeTrailingSlashesCopy(const std::wstring &path)
{
	std::wstring out = path;
	removeTrailingSlashes(out);
	return out;
}

extern std::string joinPaths(const std::string &left, const std::string &right, char delimiter)
{
	if (left.empty() && right.empty())
		return left;

	else if (left.empty())
		return right;

	else if (right.empty())
		return left;

	std::string result;
	result.reserve(left.size() + right.size() + 1);
	result.append(left);
	removeTrailingSlashes(result);
	result.append(delimiter + right);
	return result;
}

extern std::wstring joinPaths(const std::wstring &left, const std::wstring &right, wchar_t delimiter)
{
	if (left.empty() && right.empty())
		return left;

	else if (left.empty())
		return right;

	else if (right.empty())
		return left;

	std::wstring result;
	result.reserve(left.size() + right.size() + 1);
	result.append(left);
	removeTrailingSlashes(result);
	result.append(delimiter + right);
	return result;
}

extern std::string normalizePath(const std::string &path, char delimiter)
{
	std::string workpath = path;
	lang::utils::trimWhitespace(workpath);

	const bool isNetworkSharePath = (workpath.size() >= 2 && workpath[0] == '\\' && workpath[1] == '\\');

	std::vector<std::string> splitPath = lang::utils::splitString(workpath, TS_ALL_PATH_DELIMITERS);

	Int32 ignore = 0;
	std::stack<std::string*> pathTokens;
	for (auto it = splitPath.rbegin(); it != splitPath.rend(); ++it)
	{
		std::string &p = *it;

		if (p.empty() || p == ".")
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

	std::string normalized;
	normalized.reserve(path.size() / 2);

	if (isNetworkSharePath)
		normalized.append("\\\\");

	while (!pathTokens.empty())
	{
		normalized.append(*pathTokens.top());
		if (pathTokens.size() > 1)
			normalized.push_back(delimiter);
		pathTokens.pop();
	}
	return normalized;
}

extern std::wstring normalizePath(const std::wstring &path, wchar_t delimiter)
{
	TS_ASSERT(!"Not implemented.");
	return path;
}

extern std::string getDirname(const std::string &path, const std::string &delimiters)
{
	size_t pos = path.find_last_of(&delimiters[0], std::string::npos, delimiters.size());
	if (pos != std::string::npos)
		return path.substr(0, pos);
	return std::string();
}

extern std::wstring getDirname(const std::wstring &path, const std::wstring &delimiters)
{
	size_t pos = path.find_last_of(&delimiters[0], std::wstring::npos, delimiters.size());
	if (pos != std::wstring::npos)
		return path.substr(0, pos);
	return std::wstring();
}

extern std::string getBasename(const std::string &path, bool stripExtension, const std::string &delimiters)
{
	size_t extPos = stripExtension ? path.find_last_of('.', std::string::npos) : std::string::npos;

	size_t delimPos = path.find_last_of(&delimiters[0], std::string::npos, delimiters.size());
	if (delimPos != std::string::npos)
		return path.substr(delimPos + 1, extPos - 1 - delimPos);

	return path.substr(0, extPos);
}

extern std::wstring getBasename(const std::wstring &path, bool stripExtension, const std::wstring &delimiters)
{
	size_t extPos = stripExtension ? path.find_last_of(L'.', std::wstring::npos) : std::wstring::npos;

	size_t delimPos = path.find_last_of(&delimiters[0], std::wstring::npos, delimiters.size());
	if (delimPos != std::wstring::npos)
		return path.substr(delimPos + 1, extPos - 1 - delimPos);

	return path.substr(0, extPos);
}

extern std::string getExtension(const std::string &path)
{
	std::string out = path;
	removeTrailingSlashes(out);

	// Finds the last delimiter or period
	size_t pos = out.find_last_of(TS_ALL_PATH_DELIMITERS ".");
	if (pos != std::string::npos && out[pos] == '.')
		return out.substr(pos + 1);
	return std::string();
}

extern std::wstring getExtension(const std::wstring &path)
{
	std::wstring out = path;
	removeTrailingSlashes(out);

	// Finds the last delimiter or period
	size_t pos = out.find_last_of(TS_ALL_PATH_DELIMITERS_WIDE L".");
	if (pos != std::wstring::npos && out[pos] == '.')
		return out.substr(pos + 1);
	return std::wstring();
}

extern bool exists(const std::string &path)
{
	// 0 flag tests for existence only
	return access(path.c_str(), 0) == 0;
}

extern bool exists(const std::wstring &path)
{
	// 0 flag tests for existence only
	return waccess(path.c_str(), 0) == 0;
}

extern bool isFile(const std::string &path)
{
#if TS_PLATFORM == TS_WINDOWS
	return PathIsDirectoryA(path.c_str()) == 0;
#else
	struct stat s;
	if (stat(path.c_str(), &s) == 0)
	{
		if (s.st_mode & S_IFREG)
			return true;
	}
	return false;
#endif
}

extern bool isFile(const std::wstring &path)
{
#if TS_PLATFORM == TS_WINDOWS
	return PathIsDirectoryW(path.c_str()) == 0;
#else
	// Not implemented on other platforms
	TS_ASSERT(!"Wide variant not implemented on this platform.");
	return false;
#endif
}

extern bool isDirectory(const std::string &path)
{
	return !isFile(path);
}

extern bool isDirectory(const std::wstring &path)
{
#if TS_PLATFORM == TS_WINDOWS
	return !isFile(path);
#else
	// Not implemented on other platforms
	TS_ASSERT(!"Wide variant not implemented on this platform.");
	return false;
#endif
}

extern bool removeFile(const std::string &path)
{
#if TS_PLATFORM == TS_WINDOWS
	if (DeleteFileA(path.c_str()) != 0) // Non-zero marks success
		return true;

	TS_LOG_ERROR("Unable to remove file or directory. File: %s. Error: %s", path, windows::getLastErrorAsString());
#else
	if (remove(path.c_str()) == 0)
		return true;
	TS_LOG_ERROR("Unable to remove file or directory. File: %s. Error: %s", path, strerror(errno));
#endif
	return false;
}

extern bool removeFile(const std::wstring &path)
{
#if TS_PLATFORM == TS_WINDOWS
	if (DeleteFileW(path.c_str()) != 0) // Non-zero marks success
		return true;
	TS_WLOG_ERROR("Unable to remove file or directory. File: %s. Error: %s", path, windows::getLastErrorAsWideString());
#else
	TS_ASSERT(!"Wide variant not implemented on this platform.");
#endif
	return false;
}

extern std::string getExecutableDirectory()
{
	// Cache executable path
	static std::string executablePath;
	if (!executablePath.empty())
		return executablePath;

	char buffer[MAX_PATH_LENGTH] = { 0 };
#if TS_PLATFORM == TS_WINDOWS
	GetModuleFileNameA(nullptr, buffer, MAX_PATH_LENGTH);
	executablePath = getDirname(std::string(buffer));
#else
	size_t count = readlink("/proc/self/exe", buffer, MAX_PATH_LENGTH);
	if (count != -1)
		executablePath = getDirname(std::string(buffer));
#endif
	return executablePath;
}

extern std::wstring getExecutableDirectoryWide()
{
	// Cache executable path
	static std::wstring executablePath;
	if (!executablePath.empty())
		return executablePath;

#if TS_PLATFORM == TS_WINDOWS
	wchar_t buffer[MAX_PATH_LENGTH] = { 0 };
	GetModuleFileNameW(nullptr, buffer, MAX_PATH_LENGTH);
	executablePath = getDirname(std::wstring(buffer));
	return executablePath;
#else
	TS_ASSERT(!"Wide variant not implemented on this platform.");
	return std::wstring();
#endif
}

extern std::string getWorkingDirectory()
{
	char buffer[MAX_PATH_LENGTH] = { 0 };
#if TS_PLATFORM == TS_WINDOWS
	GetCurrentDirectoryA(MAX_PATH_LENGTH, buffer);
#else
	getcwd(buffer, MAX_PATH_LENGTH);
#endif
	return std::string(buffer);
}

extern std::wstring getWorkingDirectoryWide()
{
#if TS_PLATFORM == TS_WINDOWS
	wchar_t buffer[MAX_PATH_LENGTH] = { 0 };
	GetCurrentDirectoryW(MAX_PATH_LENGTH, buffer);
	return std::wstring(buffer);
#else
	TS_ASSERT(!"Wide variant not implemented on this platform.");
	return std::wstring();
#endif
}

extern void setWorkingDirectory(const std::string &path)
{
	TS_ASSERT(path.size() < MAX_PATH_LENGTH);
#if TS_PLATFORM == TS_WINDOWS
	SetCurrentDirectoryA(path.c_str());
#else
	chdir(path.c_str());
#endif
}

extern void setWorkingDirectory(const std::wstring &path)
{
	TS_ASSERT(path.size() < MAX_PATH_LENGTH);
#if TS_PLATFORM == TS_WINDOWS
	SetCurrentDirectoryW(path.c_str());
#else
	TS_ASSERT(!"Wide variant not implemented on this platform.");
#endif
}

TS_END_PACKAGE2()

