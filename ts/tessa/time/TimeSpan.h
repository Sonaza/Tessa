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

	TimeSpan(const TimeSpan &other) = default;
	TimeSpan &operator=(const TimeSpan &other) = default;
	TimeSpan(TimeSpan &&other) = default;
	TimeSpan &operator=(TimeSpan &&other) = default;

	const Int64 getSeconds() const;
	const float getSecondsAsFloat() const;
	const Int64 getMilliseconds() const;
	const Int64 getMicroseconds() const;

	static TimeSpan fromSeconds(Int64 seconds);
	static TimeSpan fromSecondsFloat(float seconds);
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
