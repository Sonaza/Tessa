#pragma once

TS_PACKAGE1(math)

template <class T>
class Vec3
{
public:
	Vec3();
	Vec3(T v);
	Vec3(T x, T y, T z);

	// Explicit cast from a vector type to another
	template <class U>
	explicit Vec3(const Vec3<U> &vector);

	// SFML conversions if using the library
#if defined(SFML_VERSION_MAJOR)

	Vec3(const sf::Vector3<T> &vector);
	operator sf::Vector3<T>();

#endif

	// Returns length of the Vector
	float length() const;

	// Returns square length of the Vector
	float squarelength() const;

	// Normalizes vector and returns current value
	Vec3 &normalize();

	// Gets normalized value without modifying current vector
	Vec3 &getNormalized();

	// Vector dot product
	float dot(const Vec3 &right);

	// Vector cross product
	Vec3 &cross(const Vec3 &right);

	// Vector components
	T x, y, z;
	
	static const Vec3<float> zero;
	static const Vec3<float> up;
	static const Vec3<float> forward;
	static const Vec3<float> right;
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

template <class T>
Vec3<T> normalize(const Vec3<T> &v);

template <class T>
float length(const Vec3<T> &v);

template <class T>
float dot(const Vec3<T> &a, const Vec3<T> &b);

template <class T>
Vec3<T> cross(const Vec3<T> &a, const Vec3<T> &b);

#include "Vec3.inl"

typedef Vec3<float>		VC3;
typedef Vec3<Int32>		VC3I;
typedef Vec3<Uint32>	VC3U;

const Vec3<float> Vec3<float>::zero(0.f, 0.f, 0.f);
const Vec3<float> Vec3<float>::up(0.f, 1.f, 0.f);
const Vec3<float> Vec3<float>::forward(0.f, 0.f, 1.f);
const Vec3<float> Vec3<float>::right(1.f, 0.f, 0.f);

TS_END_PACKAGE1()