#include "Precompiled.h"
#include "SteadyTimer.h"

TS_PACKAGE0()

SteadyTimer::SteadyTimer()
	: startTimePoint(Time::now())
{
}

TimeSpan SteadyTimer::getElapsedTime() const
{
	return Time::now() - startTimePoint;
}

TimeSpan SteadyTimer::restart()
{
	TimeSpan elapsed = getElapsedTime();
	startTimePoint = Time::now();
	return elapsed;
}

TS_END_PACKAGE0()
