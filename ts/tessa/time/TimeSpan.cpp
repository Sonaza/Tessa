#include "Precompiled.h"
#include "TimeSpan.h"

TS_PACKAGE0()

const TimeSpan TimeSpan::zero = TimeSpan();

TimeSpan::TimeSpan()
	: duration(internal_duration::zero())
{
}

TimeSpan::TimeSpan(internal_duration duration)
	: duration(duration)
{
}

const int64 TimeSpan::getSeconds() const
{
	return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
}

const float TimeSpan::getSecondsAsFloat() const
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() / 1000.f;
}

const int64 TimeSpan::getMilliseconds() const
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

const int64 TimeSpan::getMicroseconds() const
{
	return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}

const int64 TimeSpan::getNanoseconds() const
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
}

TimeSpan TimeSpan::fromSeconds(int64 seconds)
{
	return TimeSpan(std::chrono::seconds(seconds));
}

TimeSpan TimeSpan::fromSecondsFloat(float seconds)
{
	return TimeSpan(std::chrono::milliseconds(static_cast<int64>(seconds * 1000)));
}

TimeSpan TimeSpan::fromMilliseconds(int64 milliseconds)
{
	return TimeSpan(std::chrono::milliseconds(milliseconds));
}

TimeSpan TimeSpan::fromMicroseconds(int64 microseconds)
{
	return TimeSpan(std::chrono::microseconds(microseconds));
}

TimeSpan TimeSpan::fromNanoseconds(int64 nanoseconds)
{
	return TimeSpan(std::chrono::nanoseconds(nanoseconds));
}

const std::string TimeSpan::getAsString() const
{
	int64 time = getMicroseconds();

	if (math::abs(time) > 10000000)
		return TS_FMT("%0.2fs", getSecondsAsFloat());

	if (math::abs(time) > 1000)
		return TS_FMT("%0.1fms", time / 1000.f);

	return TS_FMT("%lldus", time);
}

const TimeSpan::internal_duration &TimeSpan::getDuration() const
{
	return duration;
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

TimeSpan operator""_hrs(uint64 value)
{
	return TimeSpan::fromSeconds((int64)value * 3600);
}

TimeSpan operator""_min(uint64 value)
{
	return TimeSpan::fromSeconds((int64)value * 60);
}

TimeSpan operator""_s(uint64 value)
{
	return TimeSpan::fromSeconds((int64)value);
}

TimeSpan operator""_sf(long double value)
{
	return TimeSpan::fromSecondsFloat((float)value);
}

TimeSpan operator""_ms(uint64 value)
{
	return TimeSpan::fromMilliseconds((int64)value);
}

TimeSpan operator""_us(uint64 value)
{
	return TimeSpan::fromMicroseconds(value);
}

TS_END_PACKAGE0()

