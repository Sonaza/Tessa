#pragma once

#include "ts/math/Vec2.h"
#include "ts/math/Vec3.h"
#include "ts/math/CommonMath.h"

#include <cfloat>

TS_PACKAGE1(math)

template <class T>
class Damper
{
public:
	Damper();

	// Sets current and target to value.
	Damper(const T &value, float smoothing);

	Damper(const Damper &other) = default;
	Damper &operator=(const Damper &other) = default;

	// Advance the interpolation by given delta time.
	void update(TimeSpan deltaTime);

	// Sets current and target to value.
	void reset(const T &value, float smoothing);
	
	// Zero factor will not advance interpolation.
	// Larger smoothing factor will interpolate faster.
	// Factor value range is zero to infinity.
	void setSmoothingFactor(float smoothing);
	float getSmoothingFactor() const;

	void setTarget(const T &target);
	const T &getTarget() const;

	void setValue(const T &value);
	const T &getValue() const;

	// Instantly set current value to target.
	void cutToTarget();
	
private:
	T m_currentValue;
	T m_targetValue;
	float m_smoothingFactor = FLT_MAX; 
};

#include "Damper.inl"

typedef Damper<float> FloatDamper;
typedef Damper<VC2>   VC2Damper;
typedef Damper<VC3>   VC3Damper;

TS_END_PACKAGE1()
