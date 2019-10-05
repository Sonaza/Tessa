#include "Precompiled.h"
#include "Clock.h"

TS_PACKAGE0()

Clock::Clock()
	: startTimePoint(Time::now())
{
}

TimeSpan Clock::getElapsedTime() const
{
	return Time::now() - startTimePoint;
}

TimeSpan Clock::restart()
{
	TimeSpan elapsed = getElapsedTime();
	startTimePoint = Time::now();
	return elapsed;
}

TS_END_PACKAGE0()
