
template <class T>
Vec2<T>::Vec2()
	: x(0)
	, y(0)
{
}

template <class T>
Vec2<T>::Vec2(T x, T y)
	: x(x)
	, y(y)
{
}

template <class T>
Vec2<T>::Vec2(const T v[2])
	: x(v[0])
	, y(v[1])
{
}

template <class T>
template <class U>
Vec2<T>::Vec2(const Vec2<U> &vector)
	: x(static_cast<T>(vector.x))
	, y(static_cast<T>(vector.y))
{
}

// SFML conversions if using the library
#if TS_GLOBAL_USING_SFML == TS_TRUE

template <class T>
Vec2<T>::Vec2(const sf::Vector2<T> &vector)
	: x(vector.x)
	, y(vector.y)
{
}

template <class T>
inline Vec2<T>::operator sf::Vector2<T>() const
{
	return sf::Vector2<T>(x, y);
}

#endif

template <class T>
T &Vec2<T>::operator[](SizeType index)
{
	TS_ASSERTF(index < 2, "Index out of range");
	return v[index];
}

template <class T>
const T &Vec2<T>::operator[](SizeType index) const
{
	TS_ASSERTF(index < 2, "Index out of range");
	return v[index];
}

template <class T>
inline T Vec2<T>::length() const
{
	return sqrt(x * x + y * y);
}

template <class T>
inline T Vec2<T>::squareLength() const
{
	return x * x + y * y;
}

template <class T>
inline Vec2<T> &Vec2<T>::normalize()
{
	T slen = squareLength();
	TS_ASSERT(slen != T(0));
	if (slen != T(0))
	{
		T inv = 1 / sqrt(slen);
		*this *= inv;
	}
	return *this;
}

template <class T>
inline Vec2<T> Vec2<T>::getNormalized() const
{
	return Vec2<T>(*this).normalize();
}

template <class T>
inline Vec2<T> &Vec2<T>::normalizeWithZeroFailsafe(const Vec2<T> &failsafe)
{
	T slen = squareLength();
	if (slen != T(0))
	{
		T inv = 1 / sqrt(slen);
		*this *= inv;
	}
	else
	{
		*this = failsafe;
	}
	return *this;
}

template <class T>
inline Vec2<T> Vec2<T>::getNormalizedWithZeroFailsafe(const Vec2<T> &failsafe) const
{
	return Vec2<T>(*this).normalizeWithZeroFailsafe(failsafe);
}

template <class T>
inline T Vec2<T>::dot(const Vec2<T> &other) const
{
	return x * other.x + y * other.y;
}

template <class T>
inline Vec2<T> operator*(const Vec2<T> &lhs, const Vec2<T> &rhs)
{
	return Vec2<T>(
		lhs.x * rhs.x,
		lhs.y * rhs.y
	);
}

template <class T>
inline Vec2<T> operator*(const Vec2<T> &lhs, T v)
{
	return Vec2<T>(
		lhs.x * v,
		lhs.y * v
	);
}

template <class T>
inline Vec2<T> operator*(T v, const Vec2<T> &rhs)
{
	return rhs * v;
}

template <class T>
inline Vec2<T> &operator*=(Vec2<T> &lhs, const Vec2<T> &rhs)
{
	lhs = lhs * rhs;
	return lhs;
}

template <class T>
inline Vec2<T> &operator*=(Vec2<T> &lhs, T v)
{
	lhs = lhs * v;
	return lhs;
}

template <class T>
inline Vec2<T> operator/(const Vec2<T> &lhs, const Vec2<T> &rhs)
{
	return Vec2<T>(
		lhs.x / rhs.x,
		lhs.y / rhs.y
	);
}

template <class T>
inline Vec2<T> operator/(const Vec2<T> &lhs, T v)
{
	return Vec2<T>(
		lhs.x / v,
		lhs.y / v
	);
}

template <class T>
inline Vec2<T> &operator/=(Vec2<T> &lhs, const Vec2<T> &rhs)
{
	lhs = lhs / rhs;
	return lhs;
}

template <class T>
inline Vec2<T> &operator/=(Vec2<T> &lhs, T v)
{
	lhs = lhs / v;
	return lhs;
}

template <class T>
inline Vec2<T> operator+(const Vec2<T> &lhs, const Vec2<T> &rhs)
{
	return Vec2<T>(
		lhs.x + rhs.x,
		lhs.y + rhs.y
	);
}

template <class T>
inline Vec2<T> &operator+=(Vec2<T> &lhs, const Vec2<T> &rhs)
{
	lhs = lhs + rhs;
	return lhs;
}

template <class T>
inline Vec2<T> operator-(const Vec2<T> &lhs, const Vec2<T> &rhs)
{
	return Vec2<T>(
		lhs.x - rhs.x,
		lhs.y - rhs.y
	);
}

template <class T>
inline Vec2<T> &operator-=(Vec2<T> &lhs, const Vec2<T> &rhs)
{
	lhs = lhs - rhs;
	return lhs;
}

template <class T>
inline Vec2<T> operator-(const Vec2<T> &v)
{
	return v * -1.f;
}

template <class T>
inline bool operator==(const Vec2<T> &lhs, const Vec2<T> &rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y;
}

template <class T>
inline bool operator!=(const Vec2<T> &lhs, const Vec2<T> &rhs)
{
	return (lhs.x != rhs.x) || (lhs.y != rhs.y);
}
