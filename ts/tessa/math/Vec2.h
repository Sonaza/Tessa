#pragma once

TS_PACKAGE1(math)

template <class T>
class Vec2
{
public:
	Vec2();
	Vec2(T v);
	Vec2(T x, T y);

	// Explicit cast from a vector type to another
	template <class U>
	explicit Vec2(const Vec2<U> &vector);

	// Returns length of the Vector
	float length() const;

	// Normalizes vector and returns current value
	Vec2& normalize();

	// Gets normalized value without modifying current vector
	Vec2& getNormalized();

	// Vector dot product
	float dot(const Vec2& right);

	// Vector components
	T x, y;
	
	static const Vec2<float> zero;
	static const Vec2<float> up;
	static const Vec2<float> right;
};

template <class T>
Vec2<T> operator*(const Vec2<T>& lhs, const Vec2<T>& rhs);

template <class T>
Vec2<T> operator*(const Vec2<T>& lhs, T v);

template <class T>
Vec2<T> operator*(T v, const Vec2<T>& rhs);

template <class T>
Vec2<T>& operator*=(Vec2<T>& lhs, const Vec2<T>& rhs);

template <class T>
Vec2<T>& operator*=(Vec2<T>& lhs, T v);

template <class T>
Vec2<T> operator/(const Vec2<T>& lhs, const Vec2<T>& rhs);

template <class T>
Vec2<T> operator/(const Vec2<T>& lhs, T v);

template <class T>
Vec2<T>& operator/=(Vec2<T>& lhs, const Vec2<T>& rhs);

template <class T>
Vec2<T>& operator/=(Vec2<T>& lhs, T v);

template <class T>
Vec2<T> operator+(const Vec2<T>& lhs, const Vec2<T>& rhs);

template <class T>
Vec2<T>& operator+=(Vec2<T>& lhs, const Vec2<T>& rhs);

template <class T>
Vec2<T> operator-(const Vec2<T>& lhs, const Vec2<T>& rhs);

template <class T>
Vec2<T>& operator-=(Vec2<T>& lhs, const Vec2<T>& rhs);

template <class T>
Vec2<T> operator-(const Vec2<T>& v);

template <class T>
bool operator==(const Vec2<T>& lhs, const Vec2<T>& rhs);

template <class T>
bool operator!=(const Vec2<T>& lhs, const Vec2<T>& rhs);

template <class T>
Vec2<T> normalize(const Vec2<T>& v);

template <class T>
float length(const Vec2<T>& v);

template <class T>
float dot(const Vec2<T>& a, const Vec2<T>& b);

#include "Vec2.inl"

typedef Vec2<float>		VC2;
typedef Vec2<Int32>		VC2I;
typedef Vec2<Uint32>	VC2U;

const Vec2<float> Vec2<float>::zero(0.f, 0.f);
const Vec2<float> Vec2<float>::up(0.f, 1.f);
const Vec2<float> Vec2<float>::right(1.f, 0.f);

TS_END_PACKAGE1()
