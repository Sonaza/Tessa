#pragma once

#include "ts/tessa/common/Common.h"
#include "ts/tessa/time/Time.h"

#include <chrono>

#pragma warning( disable : 4456 ) // declaration hides previous local declaration

#if TS_BUILD != TS_FINALRELEASE

#define TS_SIMPLE_ZONE()           const ts::profiling::SimpleScopedZoneTimer __simpleZoneTimer(TS_FUNCTION_LOG_SIMPLE)
#define TS_SIMPLE_ZONE_NAMED(name) const ts::profiling::SimpleScopedZoneTimer __simpleZoneTimer(TS_FUNCTION_LOG_SIMPLE, name)

#else

#define TS_SIMPLE_ZONE()           ((void)0)
#define TS_SIMPLE_ZONE_NAMED(name) ((void)0)

#endif

TS_PACKAGE1(profiling)

class SimpleScopedZoneTimer
{
public:
	SimpleScopedZoneTimer(const char *functionName, const char *zoneName = nullptr);
	~SimpleScopedZoneTimer();

private:
	Time start;
	const char *functionName = nullptr;
	const char *zoneName = nullptr;
};

TS_END_PACKAGE1()
