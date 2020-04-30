#pragma once

TS_PACKAGE1(math)

template <class T>
class Vec4
{
public:
	static const Vec4<T> zero;
	static const Vec4<T> up;
	static const Vec4<T> forward;
	static const Vec4<T> right;
	
	Vec4();
	Vec4(T x, T y, T z, T w = 1);
	Vec4(Vec3<T> v, T w = 1);
	explicit Vec4(const T v[4]);

	// Vector components
	union
	{
		T v[4];
		struct { T x, y, z, w; };
	};

	Vec4(const Vec4 &vector) = default;
	Vec4 &operator=(const Vec4 &vector) = default;

	// Explicit cast from a vector type to another
	template <class U>
	explicit Vec4(const Vec4<U> &vector);

	T &operator[](SizeType index);
	const T &operator[](SizeType index) const;

	// Returns length of the Vector
	T length() const;

	// Returns length of the Vector
	T squareLength() const;

	// Normalizes Vector and returns current value
	Vec4 &normalize();
	Vec4 getNormalized() const;
	Vec4 &normalizeWithZeroFailsafe(const Vec4<T> &failsafe);
	Vec4 getNormalizedWithZeroFailsafe(const Vec4<T> &failsafe) const;

	// Vector dot product
	T dot(const Vec4 &other) const;

	// Vector cross product
	Vec4 cross(const Vec4 &other) const;
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

#include "Vec4.inl"

typedef Vec4<float>    VC4;
typedef Vec4<double>   VC4D;
typedef Vec4<int32>    VC4I;
typedef Vec4<uint32>   VC4U;

template<> const Vec4<float> Vec4<float>::zero(0.f, 0.f, 0.f);
template<> const Vec4<float> Vec4<float>::up(0.f, 1.f, 0.f);
template<> const Vec4<float> Vec4<float>::forward(0.f, 0.f, 1.f);
template<> const Vec4<float> Vec4<float>::right(1.f, 0.f, 0.f);

template<> const Vec4<double> Vec4<double>::zero(0.0, 0.0, 0.0);
template<> const Vec4<double> Vec4<double>::up(0.0, 1.0, 0.0);
template<> const Vec4<double> Vec4<double>::forward(0.0, 0.0, 1.0);
template<> const Vec4<double> Vec4<double>::right(1.0, 0.0, 0.0);

template<> const Vec4<int32> Vec4<int32>::zero(0, 0, 0);
template<> const Vec4<int32> Vec4<int32>::up(0, 1, 0);
template<> const Vec4<int32> Vec4<int32>::forward(0, 0, 1);
template<> const Vec4<int32> Vec4<int32>::right(1, 0, 0);

template<> const Vec4<uint32> Vec4<uint32>::zero(0, 0, 0);
template<> const Vec4<uint32> Vec4<uint32>::up(0, 1, 0);
template<> const Vec4<uint32> Vec4<uint32>::forward(0, 0, 1);
template<> const Vec4<uint32> Vec4<uint32>::right(1, 0, 0);

TS_END_PACKAGE1()
