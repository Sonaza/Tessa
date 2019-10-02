#pragma once

#include "ts/tessa/math/Vec3.h"
#include "ts/tessa/math/Vec4.h"
#include "ts/tessa/math/Mat4.h"

TS_PACKAGE1(math)

class Quat
{
public:
	Quat();
	Quat(float x, float y, float z, float w);

	// Quat components
	union
	{
		float v[4];
		struct { float x, y, z, w; };
	};

	bool operator==(const Quat &other) const;
	bool operator!=(const Quat &other) const;
	
	Quat(const Quat &other);
	inline Quat &operator=(const Quat &other);

	Quat operator+(const Quat &other) const;

	Quat operator*(const Quat &other) const;
	Quat &operator*=(const Quat &other);

	Quat operator*(float s) const;
	Quat &operator*=(float s);

	/* Returns a vector rotated by the quaternion.
	 */
	VC3 getRotated(const VC3 &vector) const;

	inline float dot(const Quat &other) const;

	inline Quat getNormalized() const;
	inline Quat &normalize();
	
	//! Creates a matrix from this Quat
	void getMatrix(Mat4 &dest, const VC3 &translation = VC3::zero) const;

	/* Fast matrix retrieve, quaternion won't be normalized.
	 */
	void getMatrixFast(Mat4 &dest) const;

	/* Retrieve the quaternion inverse.
	 */ 
	Quat getInverted() const;
	Quat &invert();

	/* Linear interpolation between two quaternions.
	 * Time describes the progress where 0.f is q1 and 1.f is q2.
	 */
	Quat &lerp(Quat q1, Quat q2, float time);

	/* Spherical interpolation between two quaternions.
	 * Time describes the progress where 0.f is q1 and 1.f is q2.
	 * Threshold defines how much of the remaining interpolation will
	 * be calculated with lerp in order to avoid inaccuracies.
	 */
	Quat &slerp(Quat q1, Quat q2, float time, float threshold = 0.05f);

	/* Retrieve the current rotation as the axis of rotation and angle (in radians).
	 */
	void getAngleAxis(float &angle, VC3 &axis) const;

	/* Retrieve current euler angles (in radians).
	 */
	VC3 getEulerAngles() const;

	/* Make quaternion from Euler angles (in radians)
	*/
	static Quat makeFromEulerAngles(float x, float y, float z);
	static Quat makeFromEulerAngles(const VC3 &angles);

	/* Make quaternion that represents rotation angle (radians)
	* around the specified axis. The axis must be normalized.
	*/
	static Quat makeFromAngleAxis(float angle, const VC3 &axis);

	/* Make quaternion that represents rotation from a direction vector
	 * to the another. The vectors must be normalized.
	 */
	static Quat makeFromRotation(const VC3 &from, const VC3 &to);
};

#include "Quaternion.inl"

TS_END_PACKAGE1();
