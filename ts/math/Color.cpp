#include "Precompiled.h"
#include "Color.h"

TS_PACKAGE1(math)

template class Color<float>;

template<> const Color<float> Color<float>::black(0.f, 0.f, 0.f);
template<> const Color<float> Color<float>::white(1.f, 1.f, 1.f);
template<> const Color<float> Color<float>::red(1.f, 0.f, 0.f);
template<> const Color<float> Color<float>::green(0.f, 1.f, 0.f);
template<> const Color<float> Color<float>::blue(0.f, 0.f, 1.f);
template<> const Color<float> Color<float>::yellow(1.f, 1.f, 0.f);
template<> const Color<float> Color<float>::cyan(0.f, 1.f, 1.f);
template<> const Color<float> Color<float>::magenta(1.f, 0.f, 1.f);
template<> const Color<float> Color<float>::transparent(0.f, 0.f, 0.f, 0.f);

TS_END_PACKAGE1()
