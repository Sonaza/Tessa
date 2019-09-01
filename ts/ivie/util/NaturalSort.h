#pragma once

TS_PACKAGE2(app, util)

// extern bool naturalSort(const std::wstring &lhs, const std::wstring &rhs);

extern bool naturalSort(const String &lhs, const String &rhs);
extern bool naturalSortByExtension(const String &lhs, const String &rhs);


TS_END_PACKAGE2()
