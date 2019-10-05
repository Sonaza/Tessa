#include "Precompiled.h"
#include "NaturalSort.h"

#include "ts/file/FileUtils.h"

// #pragma warning( push, 3 )
// #include "ext/naturalorder.h"
// #pragma warning( pop )

#if TS_PLATFORM == TS_WINDOWS
	#include <shlwapi.h>
#endif

TS_PACKAGE2(app, util)

extern bool naturalSort(const String &lhs, const String &rhs)
{
	return StrCmpLogicalW(lhs.toWideString().c_str(), rhs.toWideString().c_str()) < 0;
}

// extern bool naturalSort(const String &lhs, const String &rhs)
// {
// 	return natural_compare(lhs, rhs) < 0;
// }

extern bool naturalSortByExtension(const String &lhs, const String &rhs)
{
	String extl = file::getExtension(lhs);
	String extr = file::getExtension(rhs);

	if (extl != extr)
		return naturalSort(extl, extr);

	return naturalSort(lhs, rhs);
}

TS_END_PACKAGE2()