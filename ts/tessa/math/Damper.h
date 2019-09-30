#pragma once

#include "ts/tessa/math/Vec2.h"
#include "ts/tessa/math/Vec3.h"
#include "ts/tessa/math/CommonMath.h"

TS_PACKAGE1(math)

template <class T>
class Damper
{
public:
	Damper();
	Damper(const T &target, float smoothing);

	Damper(const Damper &other) = default;
	Damper &operator=(const Damper &other) = default;

	void update(TimeSpan deltaTime);
	void reset(const T &target, float smoothing);

	void setSmoothingFactor(float smoothing);
	float getSmoothingFactor() const;

	void setTarget(const T &target);
	const T &getTarget() const;

	void setValue(const T &value);
	const T &getValue() const;

	void cutToTarget();
	
private:
	T m_currentValue;
	T m_targetValue;
	float m_smoothingFactor = 0.f;
};

#include "Damper.inl"

typedef Damper<float>	FloatDamper;
typedef Damper<VC2>		VC2Damper;
typedef Damper<VC3>		VC3Damper;

TS_END_PACKAGE1()
