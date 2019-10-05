#include "Precompiled.h"
#include "CommonMath.h"

TS_PACKAGE1(math)

bool equalsWithEpsilon(float lhs, float rhs, float epsilon)
{
	return abs(lhs - rhs) <= epsilon;
}

bool equalsWithEpsilon(double lhs, double rhs, double epsilon)
{
	return abs(lhs - rhs) <= epsilon;
}

TS_END_PACKAGE1()
