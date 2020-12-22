#include "Precompiled.h"
#include "ts/file/FileUtils.h"
#include "ts/string/StringUtils.h"

#include <stack>
#include <cstdio>
#include <regex>

TS_PACKAGE1(file)

static bool isSlash(int32_t c)
{
	return c == '/' || c == '\\';
}

extern bool hasLeadingSlash(const String &path)
{
	return !path.isEmpty() && isSlash(path.front());
}

extern bool hasTrailingSlash(const String &path)
{
	return !path.isEmpty() && isSlash(path.back());
}

extern void removeTrailingSlash(String &path)
{
	if (hasTrailingSlash(path))
		path.erase(path.end() - 1);

// 	path.erase(std::find_if(path.rbegin(), path.rbegin() + 1, [](int32_t c)
// 	{
// 		return !isSlash(c);
// 	}).base(), path.end());
}

extern String joinPaths(const String &left, const String &right, string::Character delimiter)
{
	if (left.isEmpty()) 
	{
		return right;
	}
	else if (right.isEmpty())
	{
		return left;
	}

	String result(left);
	appendPath(result, right, delimiter);
	return result;
}

extern void appendPath(String &path, const String &sectionToAppend, string::Character delimiter)
{
	if (sectionToAppend.isEmpty())
		return;

	path.reserve(path.getSize() + sectionToAppend.getSize() + 1);
	if (!hasTrailingSlash(path))
		path.append(delimiter);
	path.append(sectionToAppend);

	path = normalizePath(path, delimiter);
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

	for (BigSizeType i = 0; i < (BigSizeType)rootSegments.size(); ++i)
	{
		if (rootSegments[i] != comparedSegments[i])
			return false;
	}

	return true;
}

extern String stripRootPath(const String &path, const String &rootPath)
{
	BigSizeType rootPathSize = rootPath.getSize();
	TS_ASSERT(path.getSize() >= rootPathSize);

	if (path.getSize() == rootPathSize)
		return "";

	if (path.getSize() > rootPathSize && isSlash(path[rootPathSize]))
		rootPathSize++;

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

TS_END_PACKAGE1()

