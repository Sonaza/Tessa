#pragma once

TS_PACKAGE1(math)

template <class T>
class Vec2
{
public:
	static const Vec2<T> zero;
	static const Vec2<T> up;
	static const Vec2<T> right;

	TS_FORCEINLINE Vec2();
	TS_FORCEINLINE Vec2(T x, T y);
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
#if defined(SFML_VERSION_MAJOR)

	Vec2(const sf::Vector2<T> &vector);
	operator sf::Vector2<T>() const;

#endif

	T &operator[](SizeType index);
	const T &operator[](SizeType index) const;

	// Returns length of the Vector
	float length() const;

	// Returns square length of the Vector
	float squarelength() const;

	// Normalizes vector and returns current value
	Vec2 &normalize();

	// Gets normalized value without modifying current vector
	Vec2 &getNormalized();

	// Vector dot product
	float dot(const Vec2 &right);

	String toString() const;
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

template <class T>
Vec2<T> normalize(const Vec2<T> &v);

template <class T>
float length(const Vec2<T> &v);

template <class T>
float dot(const Vec2<T> &a, const Vec2<T> &b);

#include "Vec2.inl"

typedef Vec2<float>		VC2;
typedef Vec2<int32>		VC2I;
typedef Vec2<uint32>	VC2U;

const Vec2<float> Vec2<float>::zero(0.f, 0.f);
const Vec2<float> Vec2<float>::up(0.f, 1.f);
const Vec2<float> Vec2<float>::right(1.f, 0.f);

const Vec2<int32> Vec2<int32>::zero(0, 0);
const Vec2<int32> Vec2<int32>::up(0, 1);
const Vec2<int32> Vec2<int32>::right(1, 0);

const Vec2<uint32> Vec2<uint32>::zero(0, 0);
const Vec2<uint32> Vec2<uint32>::up(0, 1);
const Vec2<uint32> Vec2<uint32>::right(1, 0);

template<class T>
String Vec2<T>::toString() const
{
	return String().format("({}, {})", x, y);
}

template<class T>
std::basic_ostream<char> &operator<<(std::basic_ostream<char> &strm, const Vec2<T> &vec)
{
	strm << vec.toString();
	return strm;
}

template<class T>
std::basic_ostream<wchar_t> &operator<<(std::basic_ostream<wchar_t> &strm, const Vec2<T> &vec)
{
	strm << vec.toString();
	return strm;
}

TS_END_PACKAGE1()
