#pragma once

TS_PACKAGE1(math)

template <class T>
class Vec2
{
public:
	static const Vec2<T> zero;
	static const Vec2<T> up;
	static const Vec2<T> right;

	Vec2();
	Vec2(T x, T y);
	explicit Vec2(const T v[2]);

	// Vector components
	union
	{
		T v[2];
		struct { T x, y; };
	};

	Vec2(const Vec2 &vector) = default;
	Vec2 &operator=(const Vec2 &vector) = default;

	// Explicit cast from a vector type to another
	template <class U>
	explicit Vec2(const Vec2<U> &vector);

	// SFML conversions if using the library
#if TS_GLOBAL_USING_SFML == TS_TRUE

	Vec2(const sf::Vector2<T> &vector);
	operator sf::Vector2<T>() const;

#endif

	T &operator[](SizeType index);
	const T &operator[](SizeType index) const;

	// Returns length of the Vector
	T length() const;

	// Returns square length of the Vector
	T squareLength() const;

	// Normalizes the vector
	Vec2 &normalize();
	Vec2 getNormalized() const;
	Vec2 &normalizeWithZeroFailsafe(const Vec2<T> &failsafe);
	Vec2 getNormalizedWithZeroFailsafe(const Vec2<T> &failsafe) const;

	// Vector dot product
	T dot(const Vec2 &other) const;
};

template <class T>
Vec2<T> operator*(const Vec2<T> &lhs, const Vec2<T> &rhs);

template <class T>
Vec2<T> operator*(const Vec2<T> &lhs, T v);

template <class T>
Vec2<T> operator*(T v, const Vec2<T> &rhs);

template <class T>
Vec2<T> &operator*=(Vec2<T> &lhs, const Vec2<T> &rhs);

template <class T>
Vec2<T> &operator*=(Vec2<T> &lhs, T v);

template <class T>
Vec2<T> operator/(const Vec2<T> &lhs, const Vec2<T> &rhs);

template <class T>
Vec2<T> operator/(const Vec2<T> &lhs, T v);

template <class T>
Vec2<T> &operator/=(Vec2<T> &lhs, const Vec2<T> &rhs);

template <class T>
Vec2<T> &operator/=(Vec2<T> &lhs, T v);

template <class T>
Vec2<T> operator+(const Vec2<T> &lhs, const Vec2<T> &rhs);

template <class T>
Vec2<T> &operator+=(Vec2<T> &lhs, const Vec2<T> &rhs);

template <class T>
Vec2<T> operator-(const Vec2<T> &lhs, const Vec2<T> &rhs);

template <class T>
Vec2<T> &operator-=(Vec2<T> &lhs, const Vec2<T> &rhs);

template <class T>
Vec2<T> operator-(const Vec2<T> &v);

template <class T>
bool operator==(const Vec2<T> &lhs, const Vec2<T> &rhs);

template <class T>
bool operator!=(const Vec2<T> &lhs, const Vec2<T> &rhs);

#include "Vec2.inl"

typedef Vec2<float>     VC2;
typedef Vec2<double>    VC2D;
typedef Vec2<int32>     VC2I;
typedef Vec2<uint32>    VC2U;

const Vec2<float> Vec2<float>::zero(0.f, 0.f);
const Vec2<float> Vec2<float>::up(0.f, 1.f);
const Vec2<float> Vec2<float>::right(1.f, 0.f);

const Vec2<double> Vec2<double>::zero(0.0, 0.0);
const Vec2<double> Vec2<double>::up(0.0, 1.0);
const Vec2<double> Vec2<double>::right(1.0, 0.0);

const Vec2<int32> Vec2<int32>::zero(0, 0);
const Vec2<int32> Vec2<int32>::up(0, 1);
const Vec2<int32> Vec2<int32>::right(1, 0);

const Vec2<uint32> Vec2<uint32>::zero(0, 0);
const Vec2<uint32> Vec2<uint32>::up(0, 1);
const Vec2<uint32> Vec2<uint32>::right(1, 0);

TS_END_PACKAGE1()
