#pragma once

TS_PACKAGE1(math)

extern Uint16 crc16(const char *bytes, BigSizeType size);
extern Uint32 crc32(const char *bytes, BigSizeType size);
extern Uint64 crc64(const char *bytes, BigSizeType size);

TS_END_PACKAGE1()
