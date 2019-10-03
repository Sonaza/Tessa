#pragma once

#include "ts/tessa/math/Vec3.h"
#include "ts/tessa/math/Vec4.h"
#include "ts/tessa/math/Mat4.h"

TS_PACKAGE1(math)

template<class T>
class Quaternion
{
public:
	static const Quaternion zero;
	static const Quaternion identity;
	
	Quaternion();
	Quaternion(T x, T y, T z, T w);

	// Quaternion components
	union
	{
		T v[4];
		struct { T x, y, z, w; };
	};

	bool operator==(const Quaternion &other) const;
	bool operator!=(const Quaternion &other) const;
	
	Quaternion(const Quaternion &other);
	Quaternion &operator=(const Quaternion &other);

	Quaternion operator+(const Quaternion &other) const;

	Quaternion operator*(const Quaternion &other) const;
	Quaternion &operator*=(const Quaternion &other);

	Quaternion operator*(T s) const;
	Quaternion &operator*=(T s);

	/* Returns a vector rotated by the quaternion.
	 */
	Vec3<T> getRotated(const Vec3<T> &vector) const;

	T dot(const Quaternion &other) const;

	/* Normalize quaternion to account for floating point errors.
	 */
	Quaternion &normalize();
	Quaternion getNormalized() const;
	Quaternion &normalizeWithZeroFailsafe(const Quaternion &failsafe);
	Quaternion getNormalizedWithZeroFailsafe(const Quaternion &failsafe) const;
	
	/* Retrieve matrix that represents the quaternion's rotation, with an optional offset.
	 * The quaternion must be normalized before calling this function.
	 */
	void getMatrix(TMatrix4<T> &outMatrix, const Vec3<T> &offset = Vec3<T>::zero) const;

	/* Retrieve the quaternion inverse.
	 */ 
	Quaternion getInverted() const;
	Quaternion &invert();

	/* Linear interpolation between two quaternions.
	 * Time describes the progress where 0.f is q1 and 1.f is q2.
	 */
	Quaternion &lerp(Quaternion q1, Quaternion q2, T time);

	/* Spherical interpolation between two quaternions.
	 * Time describes the progress where 0.f is q1 and 1.f is q2.
	 * Threshold defines how much of the remaining interpolation will
	 * be calculated with lerp in order to avoid inaccuracies.
	 */
	Quaternion &slerp(Quaternion q1, Quaternion q2, T time, T threshold = 0.05f);

	/* Retrieve quaternion length squared.
	 */
	T squareLength() const;

	/* Retrieve quaternion length.
	 */
	T length() const;

	/* Retrieve the current rotation as the axis of rotation and angle (in radians).
	 */
	void getAngleAxis(T &angle, Vec3<T> &axis) const;

	/* Retrieve current euler angles (in radians).
	 */
	Vec3<T> getEulerAngles() const;

	/* Make quaternion from Euler angles (in radians)
	*/
	static Quaternion makeFromEulerAngles(T x, T y, T z);
	static Quaternion makeFromEulerAngles(const Vec3<T> &angles);

	/* Make quaternion that represents rotation angle (radians)
	* around the specified axis. The axis must be normalized.
	*/
	static Quaternion makeFromAngleAxis(T angle, const Vec3<T> &axis);

	/* Make quaternion that represents rotation from a direction vector
	 * to the another. The vectors must be normalized.
	 */
	static Quaternion makeFromRotation(const Vec3<T> &from, const Vec3<T> &to);
};

#include "Quaternion.inl"

const Quaternion<float> Quaternion<float>::zero(0.f, 0.f, 0.f, 0.f);
const Quaternion<float> Quaternion<float>::identity;

const Quaternion<double> Quaternion<double>::zero(0.0, 0.0, 0.0, 0.0);
const Quaternion<double> Quaternion<double>::identity;

typedef Quaternion<float> Quat;
typedef Quaternion<double> DQuat;

TS_END_PACKAGE1();
