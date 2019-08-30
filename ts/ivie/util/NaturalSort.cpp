#include "Precompiled.h"
#include "NaturalSort.h"

#include "ts/tessa/file/FileUtils.h"

#if TS_PLATFORM == TS_WINDOWS
	#include <shlwapi.h>
#else
	
#endif

TS_PACKAGE2(app, util)

extern bool NaturalSort(const std::wstring &lhs, const std::wstring &rhs)
{
	return StrCmpLogicalW(lhs.c_str(), rhs.c_str()) < 0;
}

extern bool NaturalSortByExtension(const std::wstring &lhs, const std::wstring &rhs)
{
	std::wstring extl = file::getExtension(lhs);
	std::wstring extr = file::getExtension(rhs);

	if (extl != extr)
		return NaturalSort(extl, extr);

	return NaturalSort(lhs, rhs);
}

TS_END_PACKAGE2()