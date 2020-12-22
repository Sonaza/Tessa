#include "Precompiled.h"
#include "GUID.h"

#include "ts/math/Hash.h"

#include <cstdio>
#include <cinttypes>

TS_PACKAGE0()

const GUID GUID::none = GUID(0xDEADBEEF);

GUID::GUID(uint32_t guid)
	: guid(guid)
{

}

GUID::GUID(const String &str)
	: guid(math::simpleHash32(str))
{
}

GUID::GUID(const GUID &guid)
	: guid(guid.guid)
{

}

uint32_t GUID::getUint32() const
{
	return guid;
}

const String GUID::getString() const
{
	return TS_FMT(
		"GUID('%" PRIX32 "','%" PRIX32 "')",
		(guid >> 16) & 0xFFFF, guid & 0xFFFF
	);
}

void GUID::offset(uint32_t value)
{
	guid += value;
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
