#include "Precompiled.h"
#include "Quaternion.h"

TS_PACKAGE1(math)

template class Quaternion<float>;
template class Quaternion<double>;

template<> const Quaternion<float> Quaternion<float>::zero(0.f, 0.f, 0.f, 0.f);
template<> const Quaternion<float> Quaternion<float>::identity(0.f, 0.f, 0.f, 1.f);

template<> const Quaternion<double> Quaternion<double>::zero(0.0, 0.0, 0.0, 0.0);
template<> const Quaternion<double> Quaternion<double>::identity(0.0, 0.0, 0.0, 1.0);

TS_END_PACKAGE1()
