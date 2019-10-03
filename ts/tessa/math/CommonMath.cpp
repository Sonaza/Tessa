#include "Precompiled.h"
#include "CommonMath.h"

TS_PACKAGE1(math)

bool floatEquals(float lhs, float rhs, float epsilon)
{
	return abs(lhs - rhs) <= epsilon;
}

bool floatEquals(double lhs, double rhs, double epsilon)
{
	return abs(lhs - rhs) <= epsilon;
}

TS_END_PACKAGE1()
