#include "Precompiled.h"
#include "ts/tessa/file/FileUtils.h"
#include "ts/tessa/lang/StringUtils.h"

#include <stack>
#include <cstdio>

#if TS_PLATFORM == TS_WINDOWS
	#include <shlwapi.h>
	#include <io.h> 
	#define access  _access_s
	#define waccess _waccess_s
	#pragma comment(lib, "Shlwapi.lib")
#else
	#include <unistd.h>
	#define waccess access
#endif

TS_PACKAGE2(file, utils)

namespace
{

bool isSlash(Int32 c)
{
	return c == '/' || c == '\\';
}

}

extern bool hasTrailingSlash(const std::string &path)
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

extern std::string joinPaths(const std::string &left, const std::string &right, char delimiter)
{
	std::string result;
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

extern std::string getDirname(const std::string &path, const std::string &delimiters)
{
	size_t pos = path.find_last_of(&delimiters[0], std::string::npos, delimiters.size());
	if (pos != std::string::npos)
		return path.substr(0, pos);
	return std::string();
}

extern std::string getBasename(const std::string &path, bool stripExtension, const std::string &delimiters)
{
	size_t extPos = stripExtension ? path.find_last_of('.', std::string::npos) : std::string::npos;

	size_t delimPos = path.find_last_of(&delimiters[0], std::string::npos, delimiters.size());
	if (delimPos != std::string::npos)
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

extern bool exists(const std::string &path)
{
	// 0 flag tests for existence only
	return access(path.c_str(), 0) == 0;
}

// extern bool exists(const std::wstring &path)
// {
// 	return waccess(path.c_str(), 0) == 0;
// }

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

extern bool isDirectory(const std::string &path)
{
	return !isFile(path);
}

extern bool removeFile(const std::string &path)
{
	if (remove(path.c_str()) == 0)
		return true;

	TS_LOG_ERROR("Unable to remove file or directory. File: %s. Error: %s", strerror(errno));
	return false;
}



TS_END_PACKAGE2()
