#pragma once

#include "ts/tessa/math/Vec2.h"
#include <limits>

TS_PACKAGE1(math)

enum InitializingStyle
{
	Rect_MinMaxBounds,
	Rect_PositionAndSize
};

template <class T>
class Rect
{
public:
	TS_FORCEINLINE Rect();
	TS_FORCEINLINE Rect(const math::Vec2<T> &v1, const math::Vec2<T> &v2, InitializingStyle style = Rect_MinMaxBounds);
	explicit Rect(const math::Vec2<T> v[2]);

	// Vector components
	union
	{
		math::Vec2<T> v[2];
		struct { math::Vec2<T> minbounds, maxbounds; };
	};

	Rect(const Rect &vector) = default;
	Rect &operator=(const Rect &vector) = default;

	// Explicit cast from a vector type to another
	template <class U>
	explicit Rect(const Rect<U> &other);

	// SFML conversions if using the library
#if defined(SFML_VERSION_MAJOR)

	Rect(const sf::Rect<T> &rect);
	operator sf::Rect<T>() const;

#endif

	// Tests if the bound is valid (minbound <= maxbound)
	bool isValid() const;

	// Calculate center point of the area. Returns zero vector if not valid.
	math::Vec2<T> getCenter() const;

	// Calculate the total size. Returns zero vector if not valid.
	math::Vec2<T> getSize() const;

	// Tests areas for overlap
	bool testOverlap(const Rect<T> &other);

	// Tests areas for overlap, intersection is set in the [intersection] variable.
	bool testOverlap(const Rect<T> &other, Rect<T> &intersection);

	// Tests if point is within the area.
	bool isPointWithin(const math::Vec2<T> &point);
};

template <class T>
bool operator==(const Rect<T> &lhs, const Rect<T> &rhs);

template <class T>
bool operator!=(const Rect<T> &lhs, const Rect<T> &rhs);

#include "Rect.inl"

typedef Rect<float>		FloatRect;
typedef Rect<int32>		IntRect;
typedef Rect<uint32>	UintRect;

TS_END_PACKAGE1()
