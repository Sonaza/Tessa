#pragma once

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

TS_END_PACKAGE1()
