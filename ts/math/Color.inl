
template <class T>
TS_FORCEINLINE Color<T>::Color()
	: r(0), g(0), b(0), a(1)
{
}

template <class T>
TS_FORCEINLINE Color<T>::Color(T r, T g, T b, T a)
	: r(r), g(g), b(b), a(a)
{
	TS_ASSERT(r >= 0.f && r <= 1.f);
	TS_ASSERT(g >= 0.f && g <= 1.f);
	TS_ASSERT(b >= 0.f && b <= 1.f);
	TS_ASSERT(a >= 0.f && a <= 1.f);
}

template <class T>
Color<T>::Color(const uint32 color)
	: r(((color >> 24) & 0xff) / T(255))
	, g(((color >> 16) & 0xff) / T(255))
	, b(((color >> 8)  & 0xff) / T(255))
	, a(((color >> 0)  & 0xff) / T(255))
{
	TS_ASSERT(r >= 0.f && r <= 1.f);
	TS_ASSERT(g >= 0.f && g <= 1.f);
	TS_ASSERT(b >= 0.f && b <= 1.f);
	TS_ASSERT(a >= 0.f && a <= 1.f);
}

template <class T>
Color<T>::Color(const T v[4])
	: r(v[0]), g(v[1]), b(v[2]), a(v[3])
{
	TS_ASSERT(r >= 0.f && r <= 1.f);
	TS_ASSERT(g >= 0.f && g <= 1.f);
	TS_ASSERT(b >= 0.f && b <= 1.f);
	TS_ASSERT(a >= 0.f && a <= 1.f);
}

template <class T>
uint32 Color<T>::getInteger() const
{
	return ((uint8)(math::clamp(r, T(0), T(1)) * T(255)) << 24)
	     | ((uint8)(math::clamp(g, T(0), T(1)) * T(255)) << 16)
	     | ((uint8)(math::clamp(b, T(0), T(1)) * T(255)) << 8)
	     | ((uint8)(math::clamp(a, T(0), T(1)) * T(255)) << 0);
}

// SFML conversions if using the library
#if TS_GLOBAL_USING_SFML == TS_TRUE

template <class T>
Color<T>::Color(const sf::Color &color)
	: r(color.r / T(255))
	, g(color.g / T(255))
	, b(color.b / T(255))
	, a(color.a / T(255))
{
	TS_ASSERT(r >= 0.f && r <= 1.f);
	TS_ASSERT(g >= 0.f && g <= 1.f);
	TS_ASSERT(b >= 0.f && b <= 1.f);
	TS_ASSERT(a >= 0.f && a <= 1.f);
}

template <class T>
inline Color<T>::operator sf::Color() const
{
	return sf::Color(
		(uint8)(math::clamp(r, T(0), T(1)) * T(255)),
		(uint8)(math::clamp(g, T(0), T(1)) * T(255)),
		(uint8)(math::clamp(b, T(0), T(1)) * T(255)),
		(uint8)(math::clamp(a, T(0), T(1)) * T(255))
	);
}

#endif

template <class T>
inline Color<T> operator*(const Color<T> &lhs, const Color<T> &rhs)
{
	return Color<T>(
		lhs.r * rhs.r,
		lhs.g * rhs.g,
		lhs.b * rhs.b
	);
}

template <class T>
inline Color<T> operator*(const Color<T> &lhs, T v)
{
	return Color<T>(
		lhs.r * v,
		lhs.g * v,
		lhs.b * v
	);
}

template <class T>
inline Color<T> operator*(T v, const Color<T> &rhs)
{
	return rhs * v;
}

template <class T>
inline Color<T> &operator*=(Color<T> &lhs, const Color<T> &rhs)
{
	lhs = lhs * rhs;
	return lhs;
}

template <class T>
inline Color<T> &operator*=(Color<T> &lhs, T v)
{
	lhs = lhs * v;
	return lhs;
}

template <class T>
inline Color<T> operator/(const Color<T> &lhs, const Color<T> &rhs)
{
	return Color<T>(
		lhs.r / rhs.r,
		lhs.g / rhs.g,
		lhs.b / rhs.b
	);
}

template <class T>
inline Color<T> operator/(const Color<T> &lhs, T v)
{
	return Color<T>(
		lhs.r / v,
		lhs.g / v,
		lhs.b / v
	);
}

template <class T>
inline Color<T> &operator/=(Color<T> &lhs, const Color<T> &rhs)
{
	lhs = lhs / rhs;
	return lhs;
}

template <class T>
inline Color<T> &operator/=(Color<T> &lhs, T v)
{
	lhs = lhs / v;
	return lhs;
}

template <class T>
inline Color<T> operator+(const Color<T> &lhs, const Color<T> &rhs)
{
	return Color<T>(
		lhs.r + rhs.r,
		lhs.g + rhs.g,
		lhs.b + rhs.b
	);
}

template <class T>
inline Color<T> &operator+=(Color<T> &lhs, const Color<T> &rhs)
{
	lhs = lhs + rhs;
	return lhs;
}

template <class T>
inline Color<T> operator-(const Color<T> &lhs, const Color<T> &rhs)
{
	return Color<T>(
		lhs.r - rhs.r,
		lhs.g - rhs.g,
		lhs.b - rhs.b
	);
}

template <class T>
inline Color<T> &operator-=(Color<T> &lhs, const Color<T> &rhs)
{
	lhs = lhs - rhs;
	return lhs;
}

template <class T>
inline Color<T> operator-(const Color<T> &v)
{
	return v * -T(1);
}

template <class T>
inline bool operator==(const Color<T> &lhs, const Color<T> &rhs)
{
	return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
}

template <class T>
inline bool operator!=(const Color<T> &lhs, const Color<T> &rhs)
{
	return (lhs.r != rhs.r) || (lhs.g != rhs.g) || (lhs.b != rhs.b) || (lhs.a != rhs.a);
}
