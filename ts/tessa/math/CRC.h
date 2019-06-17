#pragma once

TS_PACKAGE1(math)

extern Uint16 crc16(const Uint8 *bytes, BigSizeType size);
extern Uint32 crc32(const Uint8 *bytes, BigSizeType size);
extern Uint64 crc64(const Uint8 *bytes, BigSizeType size);

TS_END_PACKAGE1()
