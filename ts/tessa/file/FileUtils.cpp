#include "Precompiled.h"
#include "ts/tessa/file/FileUtils.h"
#include "ts/tessa/string/StringUtils.h"

#include <stack>
#include <cstdio>
#include <regex>

TS_PACKAGE1(file)

namespace
{

bool isSlash(int32 c)
{
	return c == '/' || c == '\\';
}

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

extern bool pathIsSubpath(const String &rootPath, const String &comparedPath)
{
	if (!isAbsolutePath(rootPath) || !isAbsolutePath(comparedPath))
		return false;

	const String rootNormalized = normalizePath(rootPath);
	const String comparedNormalized = normalizePath(comparedPath);

	if (rootNormalized == comparedNormalized)
		return true;

	std::vector<String> rootSegments = string::splitString(rootNormalized, TS_SYSTEM_PATH_DELIMITER);
	std::vector<String> comparedSegments = string::splitString(comparedNormalized, TS_SYSTEM_PATH_DELIMITER);

	// Not subpath if root has more segments than the other
	if (rootSegments.size() > comparedSegments.size())
		return false;

	for (BigSizeType i = 0; i < rootSegments.size(); ++i)
	{
		if (rootSegments[i] != comparedSegments[i])
			return false;
	}

	return true;
}

extern String stripRootPath(const String &path, const String &rootPath)
{
	BigSizeType rootPathSize = rootPath.getSize() + (hasTrailingSlash(rootPath) ? 0 : 1);
	if (path.getSize() < rootPathSize)
		return path;
	return path.substring(rootPathSize);
}

extern String getDirname(const String &path, const String &delimiters)
{
	if (isDirectory(path))
		return path;

	BigSizeType pos = path.findLastOf(delimiters, String::InvalidPos);
	if (pos != String::InvalidPos)
		return path.substring(0, pos);

	return path;
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

namespace
{
// Offset from Jan 1, 1601 to January 1, 1970 (start of Unix epoch) (converting from 100ns ticks to milliseconds)
const int64 UNIX_TIME_OFFSET_MILLISECONDS = 11644473600000ULL;
const int64 TICKS_PER_1KMILLISECONDS = 10000; // a tick is 100ns
}

extern TimeStamp getTimestampFromFileTime(const FileTime filetime)
{
	return (TimeStamp)filetime / TICKS_PER_1KMILLISECONDS - UNIX_TIME_OFFSET_MILLISECONDS;
}

extern FileTime getFileTimeFromTimestamp(const TimeStamp timestamp)
{
	return ((FileTime)timestamp + UNIX_TIME_OFFSET_MILLISECONDS) * TICKS_PER_1KMILLISECONDS;
}

TS_END_PACKAGE1()

