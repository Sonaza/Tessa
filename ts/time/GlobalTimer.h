#pragma once

#include "ts/time/TimeSpan.h"
#include "ts/time/Clock.h"

TS_PACKAGE0()

class GlobalTimer
{
public:
	static TimeSpan getElapsedTime();

private:
	GlobalTimer();
	static Clock globalTimer;
};

TS_END_PACKAGE0()
