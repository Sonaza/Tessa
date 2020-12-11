#pragma once

TS_PACKAGE1(math)

enum class ComponentOrder { RGBA, ARGB, ABGR, BGRA };

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

	explicit Color(const uint32 colorValue, const ComponentOrder order = ComponentOrder::RGBA);
	explicit Color(const T c[4], const ComponentOrder order = ComponentOrder::RGBA);

	uint32 getAsRGBA() const;
	uint32 getAsARGB() const;
	uint32 getAsABGR() const;
	uint32 getAsBGRA() const;
	uint32 getAsInteger(const ComponentOrder order) const;

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

template<> const Color<float> Color<float>::black;
template<> const Color<float> Color<float>::white;
template<> const Color<float> Color<float>::red;
template<> const Color<float> Color<float>::green;
template<> const Color<float> Color<float>::blue;
template<> const Color<float> Color<float>::yellow;
template<> const Color<float> Color<float>::cyan;
template<> const Color<float> Color<float>::magenta;
template<> const Color<float> Color<float>::transparent;

#endif

extern template class Color<float>;

TS_END_PACKAGE1()
