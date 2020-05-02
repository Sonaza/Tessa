#include "Precompiled.h"

#if TS_PLATFORM == TS_LINUX

#include "LinuxUtils.h"

#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

#include "ts/thread/Thread.h"
#include "ts/thread/CurrentThread.h"
#include "ts/file/FileUtils.h"

TS_PACKAGE1(linux)

extern bool openExplorerToFile(const String &filepath)
{
	TS_PRINTF("filepath %s\n", filepath);
	
	TS_ASSERTF(thread::CurrentThread::getThreadId() == Thread::getMainThread().getThreadId(), "openExplorerToFile can only be called from MainThread.");
	TS_ASSERTF(file::isAbsolutePath(filepath), "Filepath to browse to must be absolute.");
	if (!file::isAbsolutePath(filepath))
		return false;
	
	String dirpath = file::getDirname(filepath);
	if (file::isFile(dirpath))
		dirpath = file::getDirname(dirpath);
	
	if (dirpath.isEmpty())
		return false;
	
	String command = TS_FMT("xdg-open %s", dirpath);
	
	TS_PRINTF("%s\n", command);
	
	std::system(command.toUtf8().c_str());

	return true;
}

extern bool openFileWithDialog(const String &filepath)
{
	TS_ASSERTF(thread::CurrentThread::getThreadId() == Thread::getMainThread().getThreadId(), "openFileWithDialog can only be called from MainThread.");
	TS_ASSERTF(file::isAbsolutePath(filepath), "Filepath to browse to must be absolute.");
	TS_ASSERTF(file::isFile(filepath), "Filepath must be a file.");

	if (!file::isAbsolutePath(filepath) || !file::isFile(filepath))
		return false;

	// std::wstring normalizedPath = file::normalizePath(filepath).toWideString();

	return false;
}

extern bool isDebuggerAttached()
{
    char buf[4096];

    const int status_fd = ::open("/proc/self/status", O_RDONLY);
    if (status_fd == -1)
        return false;

    const ssize_t num_read = ::read(status_fd, buf, sizeof(buf) - 1);
    if (num_read <= 0)
        return false;

    buf[num_read] = '\0';
    constexpr char tracerPidString[] = "TracerPid:";
    const auto tracer_pid_ptr = ::strstr(buf, tracerPidString);
    if (!tracer_pid_ptr)
        return false;

    for (const char* characterPtr = tracer_pid_ptr + sizeof(tracerPidString) - 1; characterPtr <= buf + num_read; ++characterPtr)
    {
        if (::isspace(*characterPtr))
            continue;
        else
            return ::isdigit(*characterPtr) != 0 && *characterPtr != '0';
    }

    return false;
}

TS_END_PACKAGE1()

#endif
