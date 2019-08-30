#pragma once

#include <chrono>

TS_DECLARE0(TimeSpan)

TS_PACKAGE0()

class Time
{
	friend class TimeSpan;
public:
	typedef std::chrono::system_clock system_clock;
	typedef std::chrono::high_resolution_clock highres_clock;

	typedef highres_clock::time_point internal_time_point;
	
	static const Time zero;
	
	Time();
	explicit Time(internal_time_point time_point);

	TimeSpan fromEpoch();

	static Time now();

	Time(const Time &other) = default;
	Time &operator=(const Time &other) = default;
	Time(Time &&other) = default;
	Time &operator=(Time &&other) = default;

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
	internal_time_point time_point;
};

TS_END_PACKAGE0()
