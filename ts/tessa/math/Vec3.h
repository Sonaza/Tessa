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

	TS_FORCEINLINE Vec3();
	TS_FORCEINLINE Vec3(T x, T y, T z);
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
#if defined(SFML_VERSION_MAJOR)

	Vec3(const sf::Vector3<T> &vector);
	operator sf::Vector3<T>() const;

#endif

	T &operator[](SizeType index);
	const T &operator[](SizeType index) const;

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
typedef Vec3<int32>		VC3I;
typedef Vec3<uint32>	VC3U;

const Vec3<float> Vec3<float>::zero(0.f, 0.f, 0.f);
const Vec3<float> Vec3<float>::up(0.f, 1.f, 0.f);
const Vec3<float> Vec3<float>::forward(0.f, 0.f, 1.f);
const Vec3<float> Vec3<float>::right(1.f, 0.f, 0.f);

TS_END_PACKAGE1()