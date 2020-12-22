#include "Precompiled.h"
#include "ts/file/FileTime.h"

#include <ctime>

TS_PACKAGE1(file)

// Offset from Jan 1, 1601 to January 1, 1970 (start of Unix epoch) (converting from 100ns ticks to milliseconds)
static const int64_t UNIX_TIME_OFFSET_MILLISECONDS = 11644473600000LL;

// a tick is 100ns
static const int64_t TICKS_PER_1KMILLISECONDS = 10000; 

extern TimeStamp getTimestampFromFileTime(const FileTime filetime)
{
	return (TimeStamp)filetime / TICKS_PER_1KMILLISECONDS - UNIX_TIME_OFFSET_MILLISECONDS;
}

extern FileTime getFileTimeFromTimestamp(const TimeStamp timestamp)
{
	return ((FileTime)timestamp + UNIX_TIME_OFFSET_MILLISECONDS) * TICKS_PER_1KMILLISECONDS;
}

extern FileTime convertUnixTimeToWindowsFileTime(const timespec ts)
{
	// Converts from unix epoch time to 100 ns ticks as in Windows
	return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

TS_END_PACKAGE1()
