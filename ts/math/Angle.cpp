#include "Precompiled.h"
#include "ts/math/Angle.h"

#include "ts/math/Constants.h"

TS_PACKAGE1(math)

extern float degToRad(float degrees)
{
	return (PI / 180.f) * degrees;
}

extern double degToRad(double degrees)
{
	return (PI64 / 180.0) * degrees;
}

extern float radToDeg(float radians)
{
	return (180.f / PI) * radians;
}

extern double radToDeg(double radians)
{
	return (180.0 / PI64) * radians;
}

extern float wrapAngleRadians(float radians)
{
	radians = fmod(radians + PI, TAU);
	if (radians < 0.f)
		radians += TAU;
	return radians - PI;
}

extern double wrapAngleRadians(double radians)
{
	radians = fmod(radians + PI64, TAU64);
	if (radians < 0.0)
		radians += TAU64;
	return radians - PI64;
}

extern float wrapAngleDeg(float degrees)
{
	degrees = fmod(degrees + 180.f, 360.f);
	if (degrees < 0.f)
		degrees += 360.f;
	return degrees - 180.f;
}

extern double wrapAngleDeg(double degrees)
{
	degrees = fmod(degrees + 180.0, 360.0);
	if (degrees < 0.f)
		degrees += 360.0;
	return degrees - 180.0;
}

TS_END_PACKAGE1()
