#pragma once

TS_PACKAGE1(math)

extern float degToRad(float degrees);
extern double degToRad(double degrees);

extern float radToDeg(float radians);
extern double radToDeg(double radians);

extern float wrapAngleRad(float a);
extern double wrapAngleRad(double a);

extern float wrapAngleDeg(float a);
extern double wrapAngleDeg(double a);

TS_END_PACKAGE1()
