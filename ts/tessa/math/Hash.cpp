#include "Precompiled.h"
#include "ts/tessa/math/Hash.h"

#include "SipHash.h"

TS_PACKAGE1(math)

namespace
{
const Uint8 key[16] = { 0x87, 0xA1, 0xD6, 0x42, 0xFB, 0x18, 0x57, 0x03, 0x50, 0xA5, 0x21, 0xA6, 0x45, 0xB0, 0xBB, 0x4F, };
}

extern Uint32 simpleHash32(const char *str, BigSizeType length)
{
	Uint32 out;
	halfsiphash(reinterpret_cast<const Uint8*>(str), length, key, reinterpret_cast<Uint8*>(&out), sizeof(out));
	return out;
}

extern Uint32 simpleHash32(const std::string &str)
{
// 	size_t out = std::hash<std::string>{}(str);
	return simpleHash32(str.c_str(), str.size());
}

extern Uint64 simpleHash64(const char *str, BigSizeType length)
{
	Uint64 out;
	siphash(reinterpret_cast<const Uint8*>(str), length, key, reinterpret_cast<Uint8*>(&out), sizeof(out));
	return out;
}

extern Uint64 simpleHash64(const std::string &str)
{
	return simpleHash64(str.c_str(), str.size());
}

TS_END_PACKAGE1()
