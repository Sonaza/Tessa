#pragma once

#include "ts/tessa/Time/Time.h"
#include "ts/tessa/Time/TimeSpan.h"

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
