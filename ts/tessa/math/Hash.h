#pragma once

TS_PACKAGE1(math)

extern uint32 simpleHash32(const char *str, BigSizeType length);
extern uint32 simpleHash32(const std::string &str);
extern uint32 simpleHash32(const std::wstring &str);
extern uint32 simpleHash32(const String &str);

extern uint64 simpleHash64(const char *str, BigSizeType length);
extern uint64 simpleHash64(const std::string &str);
extern uint64 simpleHash64(const String &str);

TS_END_PACKAGE1()
