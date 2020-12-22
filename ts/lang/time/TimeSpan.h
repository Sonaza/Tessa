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

	int64_t getSeconds() const;
	float getSecondsAsFloat() const;
	int64_t getMilliseconds() const;
	int64_t getMicroseconds() const;
	int64_t getNanoseconds() const;

	static TimeSpan fromSeconds(int64_t seconds);
	static TimeSpan fromSecondsFloat(float seconds);
	static TimeSpan fromMilliseconds(int64_t milliseconds);
	static TimeSpan fromMicroseconds(int64_t microseconds);
	static TimeSpan fromNanoseconds(int64_t microseconds);

	const String getAsString() const;

	TimeSpan &operator+=(TimeSpan rhs);
	TimeSpan &operator-=(TimeSpan rhs);

	const internal_duration &getDuration() const;

private:
	internal_duration duration;
};

double operator/(TimeSpan lhs, TimeSpan rhs);

bool operator==(TimeSpan lhs, TimeSpan rhs);
bool operator!=(TimeSpan lhs, TimeSpan rhs);
bool operator<(TimeSpan lhs, TimeSpan rhs);
bool operator>(TimeSpan lhs, TimeSpan rhs);
bool operator<=(TimeSpan lhs, TimeSpan rhs);
bool operator>=(TimeSpan lhs, TimeSpan rhs);

TimeSpan operator+(TimeSpan lhs, TimeSpan rhs);
TimeSpan operator-(TimeSpan lhs, TimeSpan rhs);

TimeSpan operator""_hrs(unsigned long long int value);
TimeSpan operator""_min(unsigned long long int value);
TimeSpan operator""_s(unsigned long long int value);
TimeSpan operator""_sf(long double value);
TimeSpan operator""_ms(unsigned long long int value);
TimeSpan operator""_us(unsigned long long int value);
TimeSpan operator""_ns(unsigned long long int value);

TS_END_PACKAGE0()
