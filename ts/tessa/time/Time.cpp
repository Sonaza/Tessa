#include "Precompiled.h"
#include "Time.h"

#include "ts/tessa/time/TimeSpan.h"

TS_PACKAGE0()

const Time Time::zero = Time();

Time::Time()
	: time_point(time_point.min())
{
}

Time::Time(std::chrono::system_clock::time_point time_point)
	: time_point(time_point)
{
}

Time Time::now()
{
	return Time(std::chrono::system_clock::now());
}

TimeSpan Time::fromEpoch()
{
	return TimeSpan(time_point.time_since_epoch());
}

bool Time::operator==(Time rhs) const
{
	return time_point == rhs.time_point;
}

bool Time::operator!=(Time rhs) const
{
	return time_point != rhs.time_point;
}

bool Time::operator<(Time rhs) const
{
	return time_point < rhs.time_point;
}

bool Time::operator>(Time rhs) const
{
	return time_point > rhs.time_point;
}

bool Time::operator<=(Time rhs) const
{
	return time_point <= rhs.time_point;
}

bool Time::operator>=(Time rhs) const
{
	return time_point >= rhs.time_point;
}

Time Time::operator+(TimeSpan rhs) const
{
	return Time(time_point + rhs.duration);
}

Time Time::operator-(TimeSpan rhs) const
{
	return Time(time_point - rhs.duration);
}

Time &Time::operator+=(TimeSpan rhs)
{
	time_point += rhs.duration;
	return *this;
}

Time &Time::operator-=(TimeSpan rhs)
{
	time_point -= rhs.duration;
	return *this;
}

TimeSpan Time::operator-(Time rhs) const
{
	return TimeSpan(time_point - rhs.time_point);
}

TS_END_PACKAGE0()
