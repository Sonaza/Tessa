#include "Precompiled.h"
#include "SimpleScopedZoneTimer.h"

#include "ts/tessa/time/TimeSpan.h"
#include <cinttypes>

TS_PACKAGE1(profiling)

SimpleScopedZoneTimer::SimpleScopedZoneTimer(const char *functionName, const char *zoneName)
	: start(Time::now())
	, functionName(functionName)
	, zoneName(zoneName)
{

}

SimpleScopedZoneTimer::~SimpleScopedZoneTimer()
{
	TimeSpan elapsed = Time::now() - start;

	TS_PRINTF("[SimpleScopedZoneTimer] %s exiting: %s elapsed\n",
		zoneName != nullptr ? zoneName : functionName,
		elapsed.getAsString()
	);
}

TS_END_PACKAGE1()
