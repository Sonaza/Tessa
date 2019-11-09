#pragma once

TS_PACKAGE0()

class SteadyTimer
{
public:
	SteadyTimer();

	TimeSpan getElapsedTime() const;
	TimeSpan restart();

private:
	Time startTimePoint;
};

TS_END_PACKAGE0()
