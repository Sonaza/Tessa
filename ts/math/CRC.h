#pragma once

TS_PACKAGE1(math)

extern uint16_t crc16(const char *bytes, BigSizeType size);
extern uint32_t crc32(const char *bytes, BigSizeType size);
extern uint64_t crc64(const char *bytes, BigSizeType size);

TS_END_PACKAGE1()
