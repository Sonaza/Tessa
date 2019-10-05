#pragma once

TS_PACKAGE1(file)

// Unix timestamp format (milliseconds)
typedef uint64 TimeStamp;

// Windows filetime format (100ns ticks since Jan 1st 1601)
typedef int64 FileTime;

/* Retrieves the file modified time in Windows filetime format.
 * Returns -1 on failure (file not found or unreadable).
 */
extern FileTime getFileModifiedTime(const String &path);

/* Converts Windows filetime to Unix timestamp in milliseconds.
 */
extern TimeStamp getTimestampFromFileTime(const FileTime filetime);

/* Converts Unix timestamp (milliseconds) to Windows filetime.
 */
extern FileTime getFileTimeFromTimestamp(const TimeStamp timestamp);

/* Converts Unix filetime to Windows filetime.
 */
extern FileTime convertUnixTimeToWindowsFileTime(const timespec time);

TS_END_PACKAGE1()
