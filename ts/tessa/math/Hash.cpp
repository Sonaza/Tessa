#include "Precompiled.h"
#include "ts/tessa/math/Hash.h"

TS_PACKAGE2(math, hash)

extern Uint32 simpleHash32(const char *str)
{
	Uint32 results = 0xCACABEBE;
	for (const char *c = str; (*c) != '\0'; c++)
	{
		results = 127 * results + static_cast<unsigned char>(*c);
	}
	return results;
}

extern Uint32 simpleHash32(const std::string &str)
{
	return simpleHash32(str.c_str());
}

extern Uint64 simpleHash64(const char *str)
{
	Uint64 results = 0xDEADBEEF;
	for (const char *c = str; (*c) != '\0'; c++)
	{
		results = 127 * results + static_cast<unsigned char>(*c);
	}
	return results;
}

extern Uint64 simpleHash64(const std::string &str)
{
	return simpleHash64(str.c_str());
}

TS_END_PACKAGE2()
