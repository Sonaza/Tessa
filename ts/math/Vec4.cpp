#include "Precompiled.h"
#include "Vec4.h"

TS_PACKAGE1(math)

template class Vec4<float>;
template class Vec4<double>;
template class Vec4<int32>;
template class Vec4<uint32>;

template<> const Vec4<float> Vec4<float>::zero(0.f, 0.f, 0.f);
template<> const Vec4<float> Vec4<float>::up(0.f, 1.f, 0.f);
template<> const Vec4<float> Vec4<float>::forward(0.f, 0.f, 1.f);
template<> const Vec4<float> Vec4<float>::right(1.f, 0.f, 0.f);

template<> const Vec4<double> Vec4<double>::zero(0.0, 0.0, 0.0);
template<> const Vec4<double> Vec4<double>::up(0.0, 1.0, 0.0);
template<> const Vec4<double> Vec4<double>::forward(0.0, 0.0, 1.0);
template<> const Vec4<double> Vec4<double>::right(1.0, 0.0, 0.0);

template<> const Vec4<int32> Vec4<int32>::zero(0, 0, 0);
template<> const Vec4<int32> Vec4<int32>::up(0, 1, 0);
template<> const Vec4<int32> Vec4<int32>::forward(0, 0, 1);
template<> const Vec4<int32> Vec4<int32>::right(1, 0, 0);

template<> const Vec4<uint32> Vec4<uint32>::zero(0, 0, 0);
template<> const Vec4<uint32> Vec4<uint32>::up(0, 1, 0);
template<> const Vec4<uint32> Vec4<uint32>::forward(0, 0, 1);
template<> const Vec4<uint32> Vec4<uint32>::right(1, 0, 0);

TS_END_PACKAGE1()
