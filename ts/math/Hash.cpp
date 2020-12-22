#include "Precompiled.h"
#include "ts/math/Hash.h"

#include "SipHash.h"

TS_PACKAGE1(math)

namespace
{
const uint8_t key[16] = { 0x87, 0xA1, 0xD6, 0x42, 0xFB, 0x18, 0x57, 0x03, 0x50, 0xA5, 0x21, 0xA6, 0x45, 0xB0, 0xBB, 0x4F, };
}

extern uint32_t simpleHash32(const char *str, BigSizeType length)
{
	uint32_t out;
	halfsiphash(reinterpret_cast<const uint8_t*>(str), length, key, reinterpret_cast<uint8_t*>(&out), sizeof(out));
	return out;
}

extern uint32_t simpleHash32(const String &str)
{
	return simpleHash32(reinterpret_cast<const char *>(&str[0]), sizeof(char32_t) * str.getSize());
}

extern uint64_t simpleHash64(const char *str, BigSizeType length)
{
	uint64_t out;
	siphash(reinterpret_cast<const uint8_t*>(str), length, key, reinterpret_cast<uint8_t*>(&out), sizeof(out));
	return out;
}

extern uint64_t simpleHash64(const String &str)
{
	return simpleHash64(reinterpret_cast<const char *>(&str[0]), sizeof(char32_t) * str.getSize());
}

TS_END_PACKAGE1()
