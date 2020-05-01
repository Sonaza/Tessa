#include "Precompiled.h"
#include "Mat4.h"

TS_PACKAGE1(math)

template<> const TMatrix4<float> TMatrix4<float>::identity;
template<> const TMatrix4<double> TMatrix4<double>::identity;

TS_END_PACKAGE1()
