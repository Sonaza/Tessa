#pragma once

#include <chrono>

TS_PACKAGE0()

class TimeSpan
{
	friend class Time;
public:
	typedef std::chrono::high_resolution_clock highres_clock;
	typedef highres_clock::duration internal_duration;

	static const TimeSpan zero;

	TimeSpan();
	explicit TimeSpan(internal_duration duration);

	TimeSpan(const TimeSpan &other) = default;
	TimeSpan &operator=(const TimeSpan &other) = default;
	TimeSpan(TimeSpan &&other) = default;
	TimeSpan &operator=(TimeSpan &&other) = default;

	const int64 getSeconds() const;
	const float getSecondsAsFloat() const;
	const int64 getMilliseconds() const;
	const int64 getMicroseconds() const;
	const int64 getNanoseconds() const;

	static TimeSpan fromSeconds(int64 seconds);
	static TimeSpan fromSecondsFloat(float seconds);
	static TimeSpan fromMilliseconds(int64 milliseconds);
	static TimeSpan fromMicroseconds(int64 microseconds);
	static TimeSpan fromNanoseconds(int64 microseconds);

	const std::string getAsString() const;

	TimeSpan &operator+=(TimeSpan rhs);
	TimeSpan &operator-=(TimeSpan rhs);

	const internal_duration &getDuration() const;

private:
	internal_duration duration;
};

bool operator==(TimeSpan lhs, TimeSpan rhs);
bool operator!=(TimeSpan lhs, TimeSpan rhs);
bool operator<(TimeSpan lhs, TimeSpan rhs);
bool operator>(TimeSpan lhs, TimeSpan rhs);
bool operator<=(TimeSpan lhs, TimeSpan rhs);
bool operator>=(TimeSpan lhs, TimeSpan rhs);

TimeSpan operator+(TimeSpan lhs, TimeSpan rhs);
TimeSpan operator-(TimeSpan lhs, TimeSpan rhs);

TimeSpan operator""_hrs(uint64 value);
TimeSpan operator""_min(uint64 value);
TimeSpan operator""_s(uint64 value);
TimeSpan operator""_sf(long double value);
TimeSpan operator""_ms(uint64 value);
TimeSpan operator""_us(uint64 value);
TimeSpan operator""_ns(uint64 value);

TS_END_PACKAGE0()
