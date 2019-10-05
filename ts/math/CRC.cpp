#include "Precompiled.h"
#include "ts/math/CRC.h"

#define CRCPP_USE_CPP11
#define CRCPP_INCLUDE_ESOTERIC_CRC_DEFINITIONS
#include "ext/CRC.h"

TS_PACKAGE1(math)

extern uint16 crc16(const char *bytes, BigSizeType size)
{
	static CRC::Table<uint16, 16> table(CRC::CRC_16_XMODEM());
	return CRC::Calculate(bytes, size, table);
}

extern uint32 crc32(const char *bytes, BigSizeType size)
{
	static CRC::Table<uint32, 32> table(CRC::CRC_32());
	return CRC::Calculate(bytes, size, table);
}

extern uint64 crc64(const char *bytes, BigSizeType size)
{
	static CRC::Table<uint64, 64> table(CRC::CRC_64());
	return CRC::Calculate(bytes, size, table);
}

TS_END_PACKAGE1()
