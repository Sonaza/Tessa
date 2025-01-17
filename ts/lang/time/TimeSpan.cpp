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

int64_t TimeSpan::getSeconds() const
{
	return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
}

float TimeSpan::getSecondsAsFloat() const
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() / 1000.f;
}

int64_t TimeSpan::getMilliseconds() const
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

int64_t TimeSpan::getMicroseconds() const
{
	return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}

int64_t TimeSpan::getNanoseconds() const
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
}

TimeSpan TimeSpan::fromSeconds(int64_t seconds)
{
	return TimeSpan(std::chrono::seconds(seconds));
}

TimeSpan TimeSpan::fromSecondsFloat(float seconds)
{
	return TimeSpan(std::chrono::milliseconds(static_cast<int64_t>(seconds * 1000)));
}

TimeSpan TimeSpan::fromMilliseconds(int64_t milliseconds)
{
	return TimeSpan(std::chrono::milliseconds(milliseconds));
}

TimeSpan TimeSpan::fromMicroseconds(int64_t microseconds)
{
	return TimeSpan(std::chrono::microseconds(microseconds));
}

TimeSpan TimeSpan::fromNanoseconds(int64_t nanoseconds)
{
	return TimeSpan(std::chrono::nanoseconds(nanoseconds));
}

const String TimeSpan::getAsString() const
{
	int64_t time = getMicroseconds();

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

double operator/(TimeSpan lhs, TimeSpan rhs)
{
	TS_ASSERT(rhs != TimeSpan::zero && "Division by zero.");
	return lhs.getNanoseconds() / (double)rhs.getNanoseconds();
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

TimeSpan operator""_hrs(unsigned long long int value)
{
	return TimeSpan::fromSeconds((int64_t)value * 3600);
}

TimeSpan operator""_min(unsigned long long int value)
{
	return TimeSpan::fromSeconds((int64_t)value * 60);
}

TimeSpan operator""_s(unsigned long long int value)
{
	return TimeSpan::fromSeconds((int64_t)value);
}

TimeSpan operator""_sf(long double value)
{
	return TimeSpan::fromSecondsFloat((float)value);
}

TimeSpan operator""_ms(unsigned long long int value)
{
	return TimeSpan::fromMilliseconds((int64_t)value);
}

TimeSpan operator""_us(unsigned long long int value)
{
	return TimeSpan::fromMicroseconds(value);
}

TimeSpan operator""_ns(unsigned long long int value)
{
	return TimeSpan::fromNanoseconds(value);
}

TS_END_PACKAGE0()

