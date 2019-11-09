#pragma once

TS_PACKAGE0()

class GlobalTimer
{
public:
	static TimeSpan getElapsedTime();

private:
	GlobalTimer();
	static SteadyTimer globalTimer;
};

TS_END_PACKAGE0()
