
template <class T>
TS_FORCEINLINE Rect<T>::Rect()
	: minbounds(std::numeric_limits<T>::max(), std::numeric_limits<T>::max())
	, maxbounds(std::numeric_limits<T>::min(), std::numeric_limits<T>::min())
{
}

template <class T>
TS_FORCEINLINE Rect<T>::Rect(const math::Vec2<T> &v1, const math::Vec2<T> &v2, InitializingStyle style)
	: minbounds(v1)
	, maxbounds(style == Rect_MinMaxBounds ? v2 : v1 + v2)
{
}

template <class T>
Rect<T>::Rect(const math::Vec2<T> v[2])
	: minbounds(v[0])
	, maxbounds(v[1])
{
}

template <class T>
template <class U>
Rect<T>::Rect(const Rect<U> &other)
	: minbounds(static_cast<math::Vec2<T>>(other.minbounds))
	, maxbounds(static_cast<math::Vec2<T>>(other.maxbounds))
{
}

// SFML conversions if using the library
#if TS_GLOBAL_USING_SFML == TS_TRUE

template <class T>
Rect<T>::Rect(const sf::Rect<T> &rect)
	: minbounds(rect.left, rect.top)
	, maxbounds(rect.left + rect.width, rect.top + rect.height)
{
}

template <class T>
inline Rect<T>::operator sf::Rect<T>() const
{
	return sf::Rect<T>(
		minbounds,
		maxbounds - minbounds
	);
}

#endif

template <class T>
bool Rect<T>::isValid() const
{
	return minbounds.x < maxbounds.x && minbounds.y < maxbounds.y;
}

template <class T>
Vec2<T> Rect<T>::getCenter() const
{
	return isValid() ? (minbounds + maxbounds) / T(2) : math::Vec2<T>::zero;
}

template <class T>
Vec2<T> Rect<T>::getSize() const
{
	return isValid() ? maxbounds - minbounds : math::Vec2<T>::zero;
}

template <class T>
bool Rect<T>::testOverlap(const Rect<T> &other)
{
	Rect<T> temp;
	return testOverlap(other, temp);
}

template <class T>
bool Rect<T>::testOverlap(const Rect<T> &other, Rect<T> &intersection)
{
	if (!isValid() || !other.isValid())
		return false;

	T left   = math::max(minbounds.x, other.minbounds.x);
	T right  = math::min(maxbounds.x, other.maxbounds.x);
	T top    = math::max(minbounds.y, other.minbounds.y);
	T bottom = math::min(maxbounds.y, other.maxbounds.y);

	if ((left < right) && (top < bottom))
	{
		intersection.minbounds = math::Vec2<T>(left, top);
		intersection.maxbounds = math::Vec2<T>(right, bottom);
		return true;
	}
	return false;
}

template <class T>
bool Rect<T>::isPointWithin(const math::Vec2<T> &point)
{
	if (!isValid())
		return false;

	return point.x >= minbounds.x && point.x <= maxbounds.x &&
	       point.y >= minbounds.y && point.y <= maxbounds.y;
}

template <class T>
inline bool operator==(const Rect<T> &lhs, const Rect<T> &rhs)
{
	return lhs.minbounds == rhs.minbounds && lhs.maxbounds == rhs.maxbounds;
}

template <class T>
inline bool operator!=(const Rect<T> &lhs, const Rect<T> &rhs)
{
	return (lhs.minbounds != rhs.minbounds) || (lhs.maxbounds != rhs.maxbounds);
}
