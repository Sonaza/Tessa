#include "Precompiled.h"
#include "Vec2.h"

TS_PACKAGE1(math)

template<>
String Vec2<float>::toString() const
{
	return String().sprintf("(%0.2f, %0.2f)", x, y);
}

template<>
String Vec2<double>::toString() const
{
	return String().sprintf("(%0.2f, %0.2f)", x, y);
}

TS_END_PACKAGE1()
