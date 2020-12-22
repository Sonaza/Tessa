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
typedef Vec3<int32_t>     VC3I;
typedef Vec3<uint32_t>    VC3U;

#if TS_COMPILER != TS_MSC

template<> const Vec3<float> Vec3<float>::zero;
template<> const Vec3<float> Vec3<float>::up;
template<> const Vec3<float> Vec3<float>::forward;
template<> const Vec3<float> Vec3<float>::right;

template<> const Vec3<double> Vec3<double>::zero;
template<> const Vec3<double> Vec3<double>::up;
template<> const Vec3<double> Vec3<double>::forward;
template<> const Vec3<double> Vec3<double>::right;

template<> const Vec3<int32_t> Vec3<int32_t>::zero;
template<> const Vec3<int32_t> Vec3<int32_t>::up;
template<> const Vec3<int32_t> Vec3<int32_t>::forward;
template<> const Vec3<int32_t> Vec3<int32_t>::right;

template<> const Vec3<uint32_t> Vec3<uint32_t>::zero;
template<> const Vec3<uint32_t> Vec3<uint32_t>::up;
template<> const Vec3<uint32_t> Vec3<uint32_t>::forward;
template<> const Vec3<uint32_t> Vec3<uint32_t>::right;

#endif

extern template class Vec3<float>;
extern template class Vec3<double>;
extern template class Vec3<int32_t>;
extern template class Vec3<uint32_t>;

TS_END_PACKAGE1()
