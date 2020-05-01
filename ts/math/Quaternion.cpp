#include "Precompiled.h"
#include "Quaternion.h"

TS_PACKAGE1(math)

template<> const Quaternion<float> Quaternion<float>::zero(0.f, 0.f, 0.f, 0.f);
template<> const Quaternion<float> Quaternion<float>::identity;

template<> const Quaternion<double> Quaternion<double>::zero(0.0, 0.0, 0.0, 0.0);
template<> const Quaternion<double> Quaternion<double>::identity;

TS_END_PACKAGE1()
