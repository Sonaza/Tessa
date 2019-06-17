#include "Precompiled.h"
#include "ts/tessa/math/CRC.h"

#define CRCPP_USE_CPP11
#define CRCPP_INCLUDE_ESOTERIC_CRC_DEFINITIONS
#include "ext/CRC.h"

TS_PACKAGE1(math)

extern Uint16 crc16(const Uint8 *bytes, BigSizeType size)
{
	static CRC::Table<Uint16, 16> table(CRC::CRC_16_XMODEM());
	return CRC::Calculate(bytes, size, table);
}

extern Uint32 crc32(const Uint8 *bytes, BigSizeType size)
{
	static CRC::Table<Uint32, 32> table(CRC::CRC_32());
	return CRC::Calculate(bytes, size, table);
}

extern Uint64 crc64(const Uint8 *bytes, BigSizeType size)
{
	static CRC::Table<Uint64, 64> table(CRC::CRC_64());
	return CRC::Calculate(bytes, size, table);
}

TS_END_PACKAGE1()
