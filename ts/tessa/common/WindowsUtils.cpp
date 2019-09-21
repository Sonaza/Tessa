#include "Precompiled.h"

#if TS_PLATFORM == TS_WINDOWS

#include "WindowsUtils.h"

#include "ts/tessa/common/IncludeWindows.h"
#include "ts/tessa/thread/Thread.h"
#include "ts/tessa/thread/CurrentThread.h"
#include "ts/tessa/file/FileUtils.h"

#include <Objbase.h>
#include <Shlobj.h>

TS_PACKAGE1(windows)

namespace
{

class WinCOMInitializer
{
public:
	WinCOMInitializer()
	{
		HRESULT result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		TS_UNUSED_VARIABLE(result);
		TS_ASSERT(result == S_OK);
	}
};

WinCOMInitializer comInitializer;

}

extern String getLastErrorAsString()
{
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
		return String();

	LPWSTR messageBuffer = nullptr;
	size_t size = FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, nullptr);

	String message(messageBuffer, size);

	LocalFree(messageBuffer);

	return message;
}

extern bool openExplorerToFile(const String &filepath)
{
	TS_ASSERTF(thread::CurrentThread::getThreadId() == Thread::getMainThread().getThreadId(), "openExplorerToFile can only be called from MainThread.");
	TS_ASSERTF(file::isAbsolutePath(filepath), "Filepath to browse to must be absolute.");
	if (!file::isAbsolutePath(filepath))
		return false;

	String normalizedPath = file::normalizePath(filepath);
	LPITEMIDLIST pidl = ILCreateFromPathW(normalizedPath.toWideString().c_str());
	if (pidl == nullptr)
	{
		TS_LOG_ERROR("ILCreateFromPathW failed.");
		return false;
	}

	HRESULT result = SHOpenFolderAndSelectItems(pidl, 0, 0, 0);
	ILFree(pidl);

	if (result != S_OK)
	{
		TS_LOG_ERROR("SHOpenFolderAndSelectItems failed.");
		return false;
	}

	return true;
}

extern bool openFileWithDialog(const String &filepath)
{
	TS_ASSERTF(thread::CurrentThread::getThreadId() == Thread::getMainThread().getThreadId(), "openFileWithDialog can only be called from MainThread.");
	TS_ASSERTF(file::isAbsolutePath(filepath), "Filepath to browse to must be absolute.");
	TS_ASSERTF(file::isFile(filepath), "Filepath must be a file.");

	if (!file::isAbsolutePath(filepath) || !file::isFile(filepath))
		return false;

	std::wstring normalizedPath = file::normalizePath(filepath).toWideString();

	OPENASINFO info;
	info.pcszFile = normalizedPath.c_str();
	info.pcszClass = nullptr;
	info.oaifInFlags = OAIF_EXEC | OAIF_HIDE_REGISTRATION;

	if (SHOpenWithDialog(nullptr, &info) != S_OK)
	{
		TS_LOG_ERROR("SHOpenWithDialog failed.");
		return false;
	}

	return true;
}

extern BigSizeType convertLargeIntegerTo64bit(SizeType lowPart, SizeType highPart)
{
	LARGE_INTEGER li;
	li.LowPart = lowPart;
	li.HighPart = highPart;
	return li.QuadPart;
}


TS_END_PACKAGE1()

#endif
