#include "Precompiled.h"
#include "Vec2.h"

TS_PACKAGE1(math)

template<> const Vec2<float> Vec2<float>::zero(0.f, 0.f);
template<> const Vec2<float> Vec2<float>::up(0.f, 1.f);
template<> const Vec2<float> Vec2<float>::right(1.f, 0.f);

template<> const Vec2<double> Vec2<double>::zero(0.0, 0.0);
template<> const Vec2<double> Vec2<double>::up(0.0, 1.0);
template<> const Vec2<double> Vec2<double>::right(1.0, 0.0);

template<> const Vec2<int32> Vec2<int32>::zero(0, 0);
template<> const Vec2<int32> Vec2<int32>::up(0, 1);
template<> const Vec2<int32> Vec2<int32>::right(1, 0);

template<> const Vec2<uint32> Vec2<uint32>::zero(0, 0);
template<> const Vec2<uint32> Vec2<uint32>::up(0, 1);
template<> const Vec2<uint32> Vec2<uint32>::right(1, 0);

TS_END_PACKAGE1()