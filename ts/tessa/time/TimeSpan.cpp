#include "Precompiled.h"
#include "TimeSpan.h"

TS_PACKAGE0()

const TimeSpan TimeSpan::zero = TimeSpan();

TimeSpan::TimeSpan()
	: duration(std::chrono::system_clock::duration::zero())
{
}

TimeSpan::TimeSpan(std::chrono::system_clock::duration duration)
	: duration(duration)
{
}

const Int64 TimeSpan::getSeconds() const
{
	return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
}

const float TimeSpan::getSecondsAsFloat() const
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() / 1000.f;
}

const Int64 TimeSpan::getMilliseconds() const
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

const Int64 TimeSpan::getMicroseconds() const
{
	return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}

TimeSpan TimeSpan::fromSeconds(Int64 seconds)
{
	return TimeSpan(std::chrono::seconds(seconds));
}

TimeSpan TimeSpan::fromSecondsFloat(float seconds)
{
	return TimeSpan(std::chrono::milliseconds(static_cast<Int64>(seconds * 1000)));
}

TimeSpan TimeSpan::fromMilliseconds(Int64 milliseconds)
{
	return TimeSpan(std::chrono::milliseconds(milliseconds));
}

TimeSpan TimeSpan::fromMicroseconds(Int64 microseconds)
{
	return TimeSpan(std::chrono::microseconds(microseconds));
}

const std::string TimeSpan::getAsString() const
{
	Int64 time = getMicroseconds();

	if (time > 10000000)
		return TS_FMT("%0.2fs", getSecondsAsFloat());

	if (time > 1000)
		return TS_FMT("%0.1fms", time / 1000.f);

	return TS_FMT("%lldus", time);
}

TimeSpan &TimeSpan::operator+=(TimeSpan rhs)
{
	duration += rhs.duration;
	return *this;
}

TimeSpan &TimeSpan::operator-=(TimeSpan rhs)
{
	duration -= rhs.duration;
	return *this;
}

bool operator==(TimeSpan lhs, TimeSpan rhs)
{
	return lhs.getMicroseconds() == rhs.getMicroseconds();
}

bool operator!=(TimeSpan lhs, TimeSpan rhs)
{
	return lhs.getMicroseconds() != rhs.getMicroseconds();
}

bool operator<(TimeSpan lhs, TimeSpan rhs)
{
	return lhs.getMicroseconds() < rhs.getMicroseconds();
}

bool operator>(TimeSpan lhs, TimeSpan rhs)
{
	return lhs.getMicroseconds() > rhs.getMicroseconds();
}

bool operator<=(TimeSpan lhs, TimeSpan rhs)
{
	return lhs.getMicroseconds() <= rhs.getMicroseconds();
}

bool operator>=(TimeSpan lhs, TimeSpan rhs)
{
	return lhs.getMicroseconds() >= rhs.getMicroseconds();
}

TimeSpan operator+(TimeSpan lhs, TimeSpan rhs)
{
	return TimeSpan::fromMicroseconds(lhs.getMicroseconds() + rhs.getMicroseconds());
}

TimeSpan operator-(TimeSpan lhs, TimeSpan rhs)
{
	return TimeSpan::fromMicroseconds(lhs.getMicroseconds() - rhs.getMicroseconds());
}

TimeSpan operator""_hrs(Uint64 value)
{
	return TimeSpan::fromSeconds((Int64)value * 3600);
}

TimeSpan operator""_min(Uint64 value)
{
	return TimeSpan::fromSeconds((Int64)value * 60);
}

TimeSpan operator""_s(Uint64 value)
{
	return TimeSpan::fromSeconds((Int64)value);
}

TimeSpan operator""_sf(long double value)
{
	return TimeSpan::fromSecondsFloat((float)value);
}

TimeSpan operator""_ms(Uint64 value)
{
	return TimeSpan::fromMilliseconds((Int64)value);
}

TimeSpan operator""_us(Uint64 value)
{
	return TimeSpan::fromMicroseconds(value);
}

TS_END_PACKAGE0()

