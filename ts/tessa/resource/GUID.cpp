#include "Precompiled.h"
#include "ts/tessa/resource/GUID.h"

#include "ts/tessa/math/Hash.h"

#include <cstdio>
#include <cinttypes>

#if TS_PLATFORM == TS_WINDOWS
#define sprintf sprintf_s
#endif

TS_PACKAGE0()

const GUID GUID::none = GUID(0xDEADBEEF);

GUID::GUID(Uint32 guid)
	: guid(guid)
{

}

GUID::GUID(const std::string &str)
	: guid(math::hash::simpleHash32(str))
{
}

GUID::GUID(const GUID &guid)
	: guid(guid.guid)
{

}

Uint32 GUID::getUint32() const
{
	return guid;
}

const std::string GUID::getString() const
{
	char buffer[20] = { 0 };
	sprintf(buffer, sizeof(buffer), "GUID('%" PRIX32 "','%" PRIX32 "')", (guid >> 16) & 0xFFFF, guid & 0xFFFF);
	return std::string(buffer);
}

bool operator==(const GUID &a, const GUID &b)
{
	return a.getUint32() == b.getUint32();
}

bool operator!=(const GUID &a, const GUID &b)
{
	return !(a == b);
}

bool operator<(const GUID &a, const GUID &b)
{
	return a.getUint32() < b.getUint32();
}

TS_END_PACKAGE0()
