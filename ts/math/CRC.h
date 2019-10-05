#pragma once

TS_PACKAGE1(math)

extern uint16 crc16(const char *bytes, BigSizeType size);
extern uint32 crc32(const char *bytes, BigSizeType size);
extern uint64 crc64(const char *bytes, BigSizeType size);

TS_END_PACKAGE1()
