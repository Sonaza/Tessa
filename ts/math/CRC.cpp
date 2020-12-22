#include "Precompiled.h"
#include "ts/math/CRC.h"

#define crcpp_uint8  ::ts::uint8_t
#define crcpp_uint16 ::ts::uint16_t
#define crcpp_uint32 ::ts::uint32_t
#define crcpp_uint64 ::ts::uint64_t
#define CRCPP_USE_CPP11
#define CRCPP_INCLUDE_ESOTERIC_CRC_DEFINITIONS
#include "ext/CRC.h"

TS_PACKAGE1(math)

extern uint16_t crc16(const char *bytes, BigSizeType size)
{
	static CRC::Table<uint16_t, 16> table(CRC::CRC_16_XMODEM());
	return CRC::Calculate(bytes, size, table);
}

extern uint32_t crc32(const char *bytes, BigSizeType size)
{
	static CRC::Table<uint32_t, 32> table(CRC::CRC_32());
	return CRC::Calculate(bytes, size, table);
}

extern uint64_t crc64(const char *bytes, BigSizeType size)
{
	static CRC::Table<uint64_t, 64> table(CRC::CRC_64());
	return CRC::Calculate(bytes, size, table);
}

TS_END_PACKAGE1()
