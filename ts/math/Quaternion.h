#pragma once

#include "ts/math/Vec3.h"
#include "ts/math/Vec4.h"
#include "ts/math/Mat4.h"

TS_PACKAGE1(math)

template<class T>
class Quaternion
{
public:
	static const Quaternion<T> zero;
	static const Quaternion<T> identity;
	
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

	Quaternion operator*(const Quaternion &other) const;
	Quaternion &operator*=(const Quaternion &other);

	Quaternion operator*(T scalar) const;
	Quaternion &operator*=(T scalar);

	Quaternion operator+(const Quaternion &other) const;

	/* Returns a vector rotated by the quaternion.
	 */
	Vec2<T> getRotated(const Vec2<T> &vector) const;
	Vec3<T> getRotated(const Vec3<T> &vector) const;

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

	/* Returns dot product with another quaternion. Not very many useful use cases for this though.
	*/
	T dot(const Quaternion &other) const;

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

typedef Quaternion<float> Quat;
typedef Quaternion<double> DQuat;

#if TS_COMPILER != TS_MSC

template<> const Quaternion<float> Quaternion<float>::zero;
template<> const Quaternion<float> Quaternion<float>::identity;

template<> const Quaternion<double> Quaternion<double>::zero;
template<> const Quaternion<double> Quaternion<double>::identity;

#endif

extern template class Quaternion<float>;
extern template class Quaternion<double>;

TS_END_PACKAGE1()
