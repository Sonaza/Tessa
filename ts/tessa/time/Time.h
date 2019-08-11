#pragma once

#include <chrono>

TS_DECLARE0(TimeSpan)

TS_PACKAGE0()

class Time
{
	friend class TimeSpan;
public:
	static const Time zero;

	Time();
	explicit Time(std::chrono::system_clock::time_point time_point);

	TimeSpan fromEpoch();

	static Time now();

	bool operator==(Time rhs) const;
	bool operator!=(Time rhs) const;
	bool operator<(Time rhs) const;
	bool operator>(Time rhs) const;
	bool operator<=(Time rhs) const;
	bool operator>=(Time rhs) const;

	Time operator+(TimeSpan rhs) const;
	Time operator-(TimeSpan rhs) const;
	Time &operator+=(TimeSpan rhs);
	Time &operator-=(TimeSpan rhs);

	TimeSpan operator-(Time rhs) const;

private:
	std::chrono::system_clock::time_point time_point;
};

TS_END_PACKAGE0()
