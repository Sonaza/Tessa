#include "Precompiled.h"
#include "Vec3.h"

TS_PACKAGE1(math)

template class Vec3<float>;
template class Vec3<double>;
template class Vec3<int32>;
template class Vec3<uint32>;

template<> const Vec3<float> Vec3<float>::zero(0.f, 0.f, 0.f);
template<> const Vec3<float> Vec3<float>::up(0.f, 1.f, 0.f);
template<> const Vec3<float> Vec3<float>::forward(0.f, 0.f, 1.f);
template<> const Vec3<float> Vec3<float>::right(1.f, 0.f, 0.f);

template<> const Vec3<double> Vec3<double>::zero(0.0, 0.0, 0.0);
template<> const Vec3<double> Vec3<double>::up(0.0, 1.0, 0.0);
template<> const Vec3<double> Vec3<double>::forward(0.0, 0.0, 1.0);
template<> const Vec3<double> Vec3<double>::right(1.0, 0.0, 0.0);

template<> const Vec3<int32> Vec3<int32>::zero(0, 0, 0);
template<> const Vec3<int32> Vec3<int32>::up(0, 1, 0);
template<> const Vec3<int32> Vec3<int32>::forward(0, 0, 1);
template<> const Vec3<int32> Vec3<int32>::right(1, 0, 0);

template<> const Vec3<uint32> Vec3<uint32>::zero(0, 0, 0);
template<> const Vec3<uint32> Vec3<uint32>::up(0, 1, 0);
template<> const Vec3<uint32> Vec3<uint32>::forward(0, 0, 1);
template<> const Vec3<uint32> Vec3<uint32>::right(1, 0, 0);

TS_END_PACKAGE1()
