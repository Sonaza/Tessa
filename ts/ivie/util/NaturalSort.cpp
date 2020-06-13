#include "Precompiled.h"
#include "NaturalSort.h"

#include "ts/file/FileUtils.h"

#if TS_PLATFORM == TS_WINDOWS && 0
	#include <shlwapi.h>
#else
	#include "strnatcmp.h"
#endif

TS_PACKAGE2(app, util)

extern bool naturalSort(const String &lhs, const String &rhs)
{
#if 0 && TS_PLATFORM == TS_WINDOWS
	const std::wstring extl = lhs.toWideString();
	const std::wstring extr = rhs.toWideString();
	bool result = StrCmpLogicalW(extl.c_str(), extr.c_str()) < 0;
#else
	const std::basic_string<char32_t> tmpl = lhs.toUtf32();
	const std::basic_string<char32_t> tmpr = rhs.toUtf32();
	bool result = strnatcmp(tmpl.c_str(), tmpr.c_str()) < 0;
#endif

// 	result = result || lhs.getSize() < rhs.getSize();

	return result;
}

extern bool naturalSortFile(const viewer::ViewerImageFile &lhs, const viewer::ViewerImageFile &rhs)
{
	return naturalSort(lhs.filepath, rhs.filepath);
}

extern bool naturalSortFileByType(const viewer::ViewerImageFile &lhs, const viewer::ViewerImageFile &rhs)
{
	if (lhs.type != rhs.type)
		return naturalSort(lhs.type, rhs.type);

	return naturalSortFile(lhs, rhs);
}

extern bool naturalSortFileByLastModified(const viewer::ViewerImageFile &lhs, const viewer::ViewerImageFile &rhs)
{
	if (lhs.lastModifiedTime != rhs.lastModifiedTime)
		return lhs.lastModifiedTime < rhs.lastModifiedTime;

	return naturalSortFile(lhs, rhs);
}

TS_END_PACKAGE2()
