#pragma once

#include "ts/time/Time.h"
#include "ts/time/TimeSpan.h"

TS_PACKAGE0()

class Clock
{
public:
	Clock();

	TimeSpan getElapsedTime() const;
	TimeSpan restart();

private:
	Time startTimePoint;
};

TS_END_PACKAGE0()
