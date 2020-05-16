#pragma once

TS_PACKAGE1(math)

template <class T>
class Color
{
public:
	static const Color<T> black;
	static const Color<T> white;
	static const Color<T> red;
	static const Color<T> green;
	static const Color<T> blue;
	static const Color<T> yellow;
	static const Color<T> cyan;
	static const Color<T> magenta;
	static const Color<T> transparent;
	
	Color();
	Color(T r, T g, T b, T a = 1);
	explicit Color(const uint32 color);
	explicit Color(const T v[4]);

	uint32 getInteger() const;

	// Vector components
	union
	{
		T v[4];
		struct { T r, g, b, a; };
	};

	Color(const Color &vector) = default;
	Color &operator=(const Color &vector) = default;

	// SFML conversions if using the library
#if TS_GLOBAL_USING_SFML == TS_TRUE

	Color(const sf::Color &color);
	operator sf::Color() const;

#endif
};

template <class T>
Color<T> operator*(const Color<T> &lhs, const Color<T> &rhs);

template <class T>
Color<T> operator*(const Color<T> &lhs, T v);

template <class T>
Color<T> operator*(T v, const Color<T> &rhs);

template <class T>
Color<T> &operator*=(Color<T> &lhs, const Color<T> &rhs);

template <class T>
Color<T> &operator*=(Color<T> &lhs, T v);

template <class T>
Color<T> operator/(const Color<T> &lhs, const Color<T> &rhs);

template <class T>
Color<T> operator/(const Color<T> &lhs, T v);

template <class T>
Color<T> &operator/=(Color<T> &lhs, const Color<T> &rhs);

template <class T>
Color<T> &operator/=(Color<T> &lhs, T v);

template <class T>
Color<T> operator+(const Color<T> &lhs, const Color<T> &rhs);

template <class T>
Color<T> &operator+=(Color<T> &lhs, const Color<T> &rhs);

template <class T>
Color<T> operator-(const Color<T> &lhs, const Color<T> &rhs);

template <class T>
Color<T> &operator-=(Color<T> &lhs, const Color<T> &rhs);

template <class T>
Color<T> operator-(const Color<T> &v);

template <class T>
bool operator==(const Color<T> &lhs, const Color<T> &rhs);

template <class T>
bool operator!=(const Color<T> &lhs, const Color<T> &rhs);

#include "Color.inl"

typedef Color<float>    COL;

#if TS_COMPILER != TS_MSC

template<> const Color<float> Color<float>::zero;
template<> const Color<float> Color<float>::up;
template<> const Color<float> Color<float>::forward;
template<> const Color<float> Color<float>::right;

#endif

extern template class Color<float>;

TS_END_PACKAGE1()
