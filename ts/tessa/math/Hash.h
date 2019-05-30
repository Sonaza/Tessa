#pragma once

TS_PACKAGE2(math, hash)

extern Uint32 simpleHash32(const char *str);
extern Uint32 simpleHash32(const std::string &str);

extern Uint64 simpleHash64(const char *str);
extern Uint64 simpleHash64(const std::string &str);

TS_END_PACKAGE2()
