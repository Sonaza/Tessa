#include "Precompiled.h"
#include "GlobalTimer.h"

TS_PACKAGE0()

Clock GlobalTimer::globalTimer;

TimeSpan GlobalTimer::getElapsedTime()
{
	return globalTimer.getElapsedTime();
}

TS_END_PACKAGE0()
