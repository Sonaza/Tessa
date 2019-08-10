#pragma once

TS_PACKAGE1(math)

extern Uint32 simpleHash32(const char *str, BigSizeType length);
extern Uint32 simpleHash32(const std::string &str);
extern Uint32 simpleHash32(const std::wstring &str);

extern Uint64 simpleHash64(const char *str, BigSizeType length);
extern Uint64 simpleHash64(const std::string &str);
extern Uint64 simpleHash64(const std::wstring &str);

TS_END_PACKAGE1()
