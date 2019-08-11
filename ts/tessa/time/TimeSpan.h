#pragma once

#include <chrono>

TS_PACKAGE0()

class TimeSpan
{
	friend class Time;
public:
	static const TimeSpan zero;

	TimeSpan();
	explicit TimeSpan(std::chrono::system_clock::duration duration);

	Int64 getSeconds();
	float getSecondsAsFloat();
	Int64 getMilliseconds();
	Int64 getMicroseconds();

	static TimeSpan fromSeconds(Int64 seconds);
	static TimeSpan fromSeconds(float seconds);
	static TimeSpan fromMilliseconds(Int64 milliseconds);
	static TimeSpan fromMicroseconds(Int64 microseconds);

	TimeSpan &operator+=(TimeSpan rhs);
	TimeSpan &operator-=(TimeSpan rhs);

private:
	std::chrono::system_clock::duration duration;
};

bool operator==(TimeSpan lhs, TimeSpan rhs);
bool operator!=(TimeSpan lhs, TimeSpan rhs);
bool operator<(TimeSpan lhs, TimeSpan rhs);
bool operator>(TimeSpan lhs, TimeSpan rhs);
bool operator<=(TimeSpan lhs, TimeSpan rhs);
bool operator>=(TimeSpan lhs, TimeSpan rhs);

TimeSpan operator+(TimeSpan lhs, TimeSpan rhs);
TimeSpan operator-(TimeSpan lhs, TimeSpan rhs);

TS_END_PACKAGE0()
