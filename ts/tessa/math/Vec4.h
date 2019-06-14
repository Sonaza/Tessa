#pragma once

TS_PACKAGE1(math)

template <class T>
class Vec4
{
public:
	Vec4();
	Vec4(T v);
	Vec4(T x, T y, T z, T w = 1);
	Vec4(Vec3<T> v, T w = 1);

	// Explicit cast from a vector type to another
	template <class U>
	explicit Vec4(const Vec4<U> &vector);

	// Returns length of the Vector
	float length() const;

	// Returns length of the Vector
	float squarelength() const;

	// Normalizes Vector and returns current value
	Vec4 &normalize();

	// Gets normalized value without modifying current vector
	Vec4 &getNormalized();

	// Vector dot product
	float dot(const Vec4 &right);

	// Vector cross product
	Vec4 &cross(const Vec4 &right);

	// Vector components
	T x, y, z, w;
};

template <class T>
Vec4<T> operator*(const Vec4<T> &lhs, const Vec4<T> &rhs);

template <class T>
Vec4<T> operator*(const Vec4<T> &lhs, T v);

template <class T>
Vec4<T> operator*(T v, const Vec4<T> &rhs);

template <class T>
Vec4<T> &operator*=(Vec4<T> &lhs, const Vec4<T> &rhs);

template <class T>
Vec4<T> &operator*=(Vec4<T> &lhs, T v);

template <class T>
Vec4<T> operator/(const Vec4<T> &lhs, const Vec4<T> &rhs);

template <class T>
Vec4<T> operator/(const Vec4<T> &lhs, T v);

template <class T>
Vec4<T> &operator/=(Vec4<T> &lhs, const Vec4<T> &rhs);

template <class T>
Vec4<T> &operator/=(Vec4<T> &lhs, T v);

template <class T>
Vec4<T> operator+(const Vec4<T> &lhs, const Vec4<T> &rhs);

template <class T>
Vec4<T> &operator+=(Vec4<T> &lhs, const Vec4<T> &rhs);

template <class T>
Vec4<T> operator-(const Vec4<T> &lhs, const Vec4<T> &rhs);

template <class T>
Vec4<T> &operator-=(Vec4<T> &lhs, const Vec4<T> &rhs);

template <class T>
Vec4<T> operator-(const Vec4<T> &v);

template <class T>
bool operator==(const Vec4<T> &lhs, const Vec4<T> &rhs);

template <class T>
bool operator!=(const Vec4<T> &lhs, const Vec4<T> &rhs);

template <class T>
Vec4<T> normalize(const Vec4<T> &v);

template <class T>
float length(const Vec4<T> &v);

template <class T>
float dot(const Vec4<T> &a, const Vec4<T> &b);

template <class T>
Vec4<T> cross(const Vec4<T> &a, const Vec4<T> &b);

#include "Vec4.inl"

typedef Vec4<float>		VC4;
typedef Vec4<Int32>		VC4I;
typedef Vec4<Uint32>	VC4U;

TS_END_PACKAGE1()