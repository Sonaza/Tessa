#pragma once

TS_PACKAGE0()

class SteadyTimer
{
public:
	SteadyTimer();

	// Returns currently elapsed time
	TimeSpan getElapsedTime() const;

	// Restart timer and return elapsed time before reset
	TimeSpan restart();

	// Sets the timer to specified elapsed time
	void set(const TimeSpan elapsed);

private:
	Time startTimePoint;
};

TS_END_PACKAGE0()
