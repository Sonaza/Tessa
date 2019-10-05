#include "Precompiled.h"

#if TS_PLATFORM == TS_WINDOWS

#include "ts/file/FileTime.h"
#include "ts/lang/common/IncludeWindows.h"

TS_PACKAGE1(file)

extern FileTime getFileModifiedTime(const String &path)
{
	WIN32_FILE_ATTRIBUTE_DATA data;
	if (GetFileAttributesExW(path.toWideString().c_str(), GetFileExInfoStandard, &data))
	{
		if (data.nFileSizeLow == 0 && data.nFileSizeHigh == 0)
			return -1;

		FILETIME ft = data.ftLastWriteTime;
		LARGE_INTEGER li;
		li.LowPart = ft.dwLowDateTime;
		li.HighPart = ft.dwHighDateTime;
		return li.QuadPart;
	}
	return -1;
}

TS_END_PACKAGE1()

#endif
