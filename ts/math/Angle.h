#pragma once

TS_PACKAGE1(math)

extern float degToRad(float degrees);
extern float radToDeg(float radians);

extern float wrapAngleRad(float a);
extern float wrapAngleDeg(float a);

TS_END_PACKAGE1()
