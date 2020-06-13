#include "Precompiled.h"

#if TS_PLATFORM == TS_WINDOWS

#include "ts/file/FileUtils.h"
#include "ts/string/StringUtils.h"

#include "ts/lang/common/IncludeWindows.h"
#include <shlwapi.h>
#include <shellapi.h>

#define MAX_PATH_LENGTH MAX_PATH

TS_PACKAGE1(file)

extern bool isAbsolutePath(const String &path)
{
	return PathIsRelativeW(path.toWideString().c_str()) == FALSE;
}

extern String normalizePath(const String &path, string::Character delimiter)
{
	TS_ASSERT(path.getSize() <= MAX_PATH_LENGTH);
	if (path.getSize() > MAX_PATH_LENGTH)
		return "";

	// PathCchCanonicalize would be a better alternative
	// but it is only supported from Windows 8 onwards.
	wchar_t buffer[MAX_PATH_LENGTH] = {};
	if (PathCanonicalizeW(buffer, path.toWideString().c_str()))
		return String(buffer);

	return "";
}

extern bool exists(const String &path)
{
	return PathFileExistsW(path.toWideString().c_str()) == TRUE;
}

extern bool isFile(const String &path)
{
	return !isDirectory(path);
}

extern bool isDirectory(const String &path)
{
	return PathIsDirectoryW(path.toWideString().c_str()) != FALSE;
}

extern bool removeFile(const String &path)
{
	if (DeleteFileW(path.toWideString().c_str()) == FALSE)
	{
		TS_LOG_ERROR("Unable to remove file or directory. File: %s. Error: %s",
			path, windows::getLastErrorAsString());
		return false;
	}
	return true;
}

extern String getExecutableDirectory()
{
	// Cache executable path
	static String executablePath;
	if (!executablePath.isEmpty())
		return executablePath;

	wchar_t buffer[MAX_PATH_LENGTH] = {};
	GetModuleFileNameW(nullptr, buffer, MAX_PATH_LENGTH);
	executablePath = getDirname(String(buffer));
	return executablePath;
}

extern String getWorkingDirectory()
{
	wchar_t buffer[MAX_PATH_LENGTH] = {};
	GetCurrentDirectoryW(MAX_PATH_LENGTH, buffer);
	return String(buffer);
}

extern void setWorkingDirectory(const String &path)
{
	TS_ASSERT(path.getSize() <= MAX_PATH_LENGTH);
	SetCurrentDirectoryW(path.toWideString().c_str());
}

extern String getShellFileType(const String &path)
{
	SHFILEINFOW shfi;
	SHGetFileInfoW(path.toWideString().c_str(), 0, &shfi, sizeof(shfi), SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
	return String(shfi.szTypeName);
}

TS_END_PACKAGE1()

#endif
