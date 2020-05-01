#include "Precompiled.h"
#include "Mat4.h"

TS_PACKAGE1(math)

template class TMatrix4<float>;
template class TMatrix4<double>;

template<> const TMatrix4<float> TMatrix4<float>::identity(
	1.f, 0.f, 0.f, 0.f,
	0.f, 1.f, 0.f, 0.f,
	0.f, 0.f, 1.f, 0.f,
	0.f, 0.f, 0.f, 1.f);

template<> const TMatrix4<double> TMatrix4<double>::identity(
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0);

TS_END_PACKAGE1()
