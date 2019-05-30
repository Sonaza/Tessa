#include "Precompiled.h"
#include "ts/tessa/math/Angle.h"

#include "ts/tessa/math/Constants.h"

TS_PACKAGE1(math)

extern float degToRad(float degrees)
{
	return (PI / 180.f) * degrees;
}

extern float radToDeg(float radians)
{
	return (180.f / PI) * radians;
}

extern float wrapAngleRadians(float radians)
{
	radians = fmod(radians + PI, TAU);
	if (radians < 0.f)
		radians += TAU;
	return radians - PI;
}

extern float wrapAngleDeg(float degrees)
{
	degrees = fmod(degrees + 180.f, 360.f);
	if (degrees < 0.f)
		degrees += 360.f;
	return degrees - 180.f;
}

TS_END_PACKAGE1()
