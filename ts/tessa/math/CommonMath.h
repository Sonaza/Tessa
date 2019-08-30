#pragma once

#include <cmath>

TS_PACKAGE1(math)

template<class T, class... Args>
T min(T a, T b, Args... args)
{
	return a < b ? min(a, args...) : min(b, args...);
}

template<class T>
T min(T a, T b)
{
	return a < b ? a : b;
}

template<class T, class... Args>
T max(T a, T b, Args... args)
{
	return a > b ? min(a, args...) : min(b, args...);
}

template<class T>
T max(T a, T b)
{
	return a > b ? a : b;
}

template<class T>
T clamp(T value, T minimum, T maximum)
{
	return min(maximum, max(minimum, value));
}

template<class T>
T abs(T value)
{
	return value >= 0 ? value : -value;
}

template<class T>
int32 sign(T value)
{
	return value >= 0 ? 1 : -1;
}

template<class T>
T ceil(T value)
{
	return std::ceil(value);
}

template<class T>
T floor(T value)
{
	return std::floor(value);
}

template<class T>
T greatestCommonDivisor(T a, T b)
{
	return (b == 0) ? a : greatestCommonDivisor(b, a % b);
}

TS_END_PACKAGE1()
