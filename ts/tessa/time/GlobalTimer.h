#pragma once

#include "ts/tessa/time/TimeSpan.h"
#include "ts/tessa/time/Clock.h"

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
