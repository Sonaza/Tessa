#pragma once

TS_PACKAGE1(math)

template <class T>
class Vec3
{
public:
	static const Vec3<T> zero;
	static const Vec3<T> up;
	static const Vec3<T> forward;
	static const Vec3<T> right;

	Vec3();
	Vec3(T x, T y, T z);
	explicit Vec3(const T v[3]);

	// Vector components
	union
	{
		T v[3];
		struct { T x, y, z; };
	};

	Vec3(const Vec3 &vector) = default;
	Vec3 &operator=(const Vec3 &vector) = default;

	// Explicit cast from a vector type to another
	template <class U>
	explicit Vec3(const Vec3<U> &vector);

	// SFML conversions if using the library
#if TS_GLOBAL_USING_SFML == TS_TRUE

	Vec3(const sf::Vector3<T> &vector);
	operator sf::Vector3<T>() const;

#endif

	T &operator[](SizeType index);
	const T &operator[](SizeType index) const;

	// Returns length of the Vector
	T length() const;

	// Returns square length of the Vector
	T squareLength() const;

	// Normalizes vector and returns current value
	Vec3 &normalize();
	Vec3 getNormalized() const;
	Vec3 &normalizeWithZeroFailsafe(const Vec3<T> &failsafe);
	Vec3 getNormalizedWithZeroFailsafe(const Vec3<T> &failsafe) const;

	// Vector dot product
	T dot(const Vec3 &other) const;

	// Vector cross product
	Vec3 cross(const Vec3 &other) const;
};

template <class T>
Vec3<T> operator*(const Vec3<T> &lhs, const Vec3<T> &rhs);

template <class T>
Vec3<T> operator*(const Vec3<T> &lhs, T v);

template <class T>
Vec3<T> operator*(T v, const Vec3<T> &rhs);

template <class T>
Vec3<T> &operator*=(Vec3<T> &lhs, const Vec3<T> &rhs);

template <class T>
Vec3<T> &operator*=(Vec3<T> &lhs, T v);

template <class T>
Vec3<T> operator/(const Vec3<T> &lhs, const Vec3<T> &rhs);

template <class T>
Vec3<T> operator/(const Vec3<T> &lhs, T v);

template <class T>
Vec3<T> &operator/=(Vec3<T> &lhs, const Vec3<T> &rhs);

template <class T>
Vec3<T> &operator/=(Vec3<T> &lhs, T v);

template <class T>
Vec3<T> operator+(const Vec3<T> &lhs, const Vec3<T> &rhs);

template <class T>
Vec3<T> &operator+=(Vec3<T> &lhs, const Vec3<T> &rhs);

template <class T>
Vec3<T> operator-(const Vec3<T> &lhs, const Vec3<T> &rhs);

template <class T>
Vec3<T> &operator-=(Vec3<T> &lhs, const Vec3<T> &rhs);

template <class T>
Vec3<T> operator-(const Vec3<T> &v);

template <class T>
bool operator==(const Vec3<T> &lhs, const Vec3<T> &rhs);

template <class T>
bool operator!=(const Vec3<T> &lhs, const Vec3<T> &rhs);

#include "Vec3.inl"

typedef Vec3<float>     VC3;
typedef Vec3<double>    VC3D;
typedef Vec3<int32>     VC3I;
typedef Vec3<uint32>    VC3U;

template<> const Vec3<float> Vec3<float>::zero(0.f, 0.f, 0.f);
template<> const Vec3<float> Vec3<float>::up(0.f, 1.f, 0.f);
template<> const Vec3<float> Vec3<float>::forward(0.f, 0.f, 1.f);
template<> const Vec3<float> Vec3<float>::right(1.f, 0.f, 0.f);

template<> const Vec3<double> Vec3<double>::zero(0.0, 0.0, 0.0);
template<> const Vec3<double> Vec3<double>::up(0.0, 1.0, 0.0);
template<> const Vec3<double> Vec3<double>::forward(0.0, 0.0, 1.0);
template<> const Vec3<double> Vec3<double>::right(1.0, 0.0, 0.0);

template<> const Vec3<int32> Vec3<int32>::zero(0, 0, 0);
template<> const Vec3<int32> Vec3<int32>::up(0, 1, 0);
template<> const Vec3<int32> Vec3<int32>::forward(0, 0, 1);
template<> const Vec3<int32> Vec3<int32>::right(1, 0, 0);

template<> const Vec3<uint32> Vec3<uint32>::zero(0, 0, 0);
template<> const Vec3<uint32> Vec3<uint32>::up(0, 1, 0);
template<> const Vec3<uint32> Vec3<uint32>::forward(0, 0, 1);
template<> const Vec3<uint32> Vec3<uint32>::right(1, 0, 0);

TS_END_PACKAGE1()
