#pragma once

#include "ts/tessa/string/String.h"
#include "ts/tessa/math/Vec2.h"
#include "ts/tessa/math/Vec3.h"
#include "ts/tessa/math/Vec4.h"

TS_PACKAGE1(math)

template<class T>
String VectorToString(const Vec2<T> &v)
{
	return String().format("Vec2({}, {})", v.x, v.y);
}

template<class T>
std::basic_ostream<char> &operator<<(std::basic_ostream<char> &strm, const Vec2<T> &v)
{
	strm << VectorToString(v);
	return strm;
}

template<class T>
std::basic_ostream<wchar_t> &operator<<(std::basic_ostream<wchar_t> &strm, const Vec2<T> &v)
{
	strm << VectorToString(v);
	return strm;
}

template<class T>
String VectorToString(const Vec3<T> &v)
{
	return String().format("Vec3({}, {}, {})", v.x, v.y, v.z);
}

template<class T>
std::basic_ostream<char> &operator<<(std::basic_ostream<char> &strm, const Vec3<T> &v)
{
	strm << VectorToString(v);
	return strm;
}

template<class T>
std::basic_ostream<wchar_t> &operator<<(std::basic_ostream<wchar_t> &strm, const Vec3<T> &v)
{
	strm << VectorToString(v);
	return strm;
}

template<class T>
String VectorToString(const Vec4<T> &v)
{
	return String().format("Vec4({}, {}, {}, {})", v.x, v.y, v.z, v.w);
}

template<class T>
std::basic_ostream<char> &operator<<(std::basic_ostream<char> &strm, const Vec4<T> &v)
{
	strm << VectorToString(v);
	return strm;
}

template<class T>
std::basic_ostream<wchar_t> &operator<<(std::basic_ostream<wchar_t> &strm, const Vec4<T> &v)
{
	strm << VectorToString(v);
	return strm;
}

TS_END_PACKAGE1()
