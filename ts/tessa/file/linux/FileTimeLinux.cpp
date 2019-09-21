#include "Precompiled.h"

#if TS_PLATFORM == TS_LINUX

#include "ts/tessa/file/FileTime.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

TS_PACKAGE1(file)

extern FileTime getFileModifiedTime(const String &path)
{
	struct stat st;
	if (stat(path.toUtf8().c_str(), &st) == 0)
		return getFileTimeFromTimestamp(convertUnixTimeToWindowsFileTime(st.st_mtim));
	
	return 0;
}

TS_END_PACKAGE1()

#endif
