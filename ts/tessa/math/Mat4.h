#pragma once

#include "ts/tessa/math/Vec2.h"
#include "ts/tessa/math/Vec3.h"
#include "ts/tessa/math/Vec4.h"

#if TS_GLOBAL_USING_SFML == TS_TRUE
#include "SFML/Graphics/Transform.hpp"
#endif

#include <cmath>
#include "ts/tessa/math/Angle.h"

TS_PACKAGE1(math)

template<class T>
class TMatrix4
{
	template<class> friend class Quaternion;

public:
	static const TMatrix4 identity;

	// Constructs an identity matrix
	TMatrix4();

	// Constructs from 3x3 matrix by expanding to 4x4
	TMatrix4(T m00, T m01, T m02,
	         T m10, T m11, T m12,
	         T m20, T m21, T m22);

	// Construct directly from individual components
	TMatrix4(T m00, T m01, T m02, T m03,
	         T m10, T m11, T m12, T m13,
	         T m20, T m21, T m22, T m23,
	         T m30, T m31, T m32, T m33);

	// Construct from component array (note order is different compared to other constructors)
	explicit TMatrix4(T m[16]);

	// Default copy operations
	TMatrix4(const TMatrix4 &other) = default;
	TMatrix4 &operator=(const TMatrix4 &other) = default;

	// Retrieve current matrix
	const T *getMatrix() const;

	// Calculate inverse matrix
	TMatrix4 &invert();
	TMatrix4 getInverse() const;

	// Matrix multiplication
	TMatrix4 &combine(const TMatrix4 &matrix);

	// Translate matrix by offset
	TMatrix4 &translate(const Vec2<T> &offset);
	TMatrix4 &translate(T x, T y);
	TMatrix4 &translate(const Vec3<T> &offset);

	// Rotates matrix by given amount on Z axis (for 2D graphics)
	TMatrix4 &rotate(T degrees, const Vec2<T> &center = Vec2<T>::zero);

	// Rotate matrix by amount on specified axis
	TMatrix4 &rotate(T degrees, const Vec3<T> &axis);

	// Scale the matrix by amount
	TMatrix4 &scale(const Vec2<T> &scale, const Vec2<T> &center = Vec2<T>::zero);
	TMatrix4 &scale(T scaleX, T scaleY, T centerX = T(0.0), T centerY = T(0.0));
	TMatrix4 &scale(const Vec3<T> &scale);

	// Transpose the matrix
	TMatrix4 &transpose();

	// Transforms a vector point by the current matrix
	Vec2<T> transformPoint(const Vec2<T> &point) const;
	Vec3<T> transformPoint(const Vec3<T> &point) const;
	Vec4<T> transformPoint(const Vec4<T> &point) const;

	// Calculate a lookat matrix 
	static TMatrix4 makeLookAt(const Vec3<T> &eye, const Vec3<T> &target, const Vec3<T> &up);

	// Calculates 3D projection matrix
	static TMatrix4 makePerspective(T fovInDegrees, T aspect, T znear, T zfar);

	// Calculates orthographic projection matrix
	static TMatrix4 makeOrtho(T left, T right,
		T bottom, T top, T znear = T(-1.9), T zfar = T(1.0));

#if TS_GLOBAL_USING_SFML == TS_TRUE
	TMatrix4(const sf::Transform &transform);
	explicit operator sf::Transform() const;
#endif

	// Row utility class for bracket access
	class Row
	{
	public:
		Row(T a, T b, T c, T d)
		{
			m_row[0] = a;
			m_row[1] = b;
			m_row[2] = c;
			m_row[3] = d;
		}

		T operator[](unsigned int i) const
		{
			TS_ASSERT(i >= 0 && i < 4);
			return m_row[i];
		}

	private:
		T m_row[4];
	};

	// Overload brackets to retrieve a single row
	const Row operator[](unsigned int i)
	{
		TS_ASSERT(i >= 0 && i < 4);
		return Row(
			m_matrix[i * 4 + 0],
			m_matrix[i * 4 + 1],
			m_matrix[i * 4 + 2],
			m_matrix[i * 4 + 3]
		);
	}

private:
	T m_matrix[16];
};

template<class T>
bool operator==(const TMatrix4<T> &lhs, const TMatrix4<T> &rhs);
template<class T>
bool operator!=(const TMatrix4<T> &lhs, const TMatrix4<T> &rhs);

template<class T>
TMatrix4<T> operator*(const TMatrix4<T> &lhs, const TMatrix4<T> &rhs);

#include "Mat4.inl"

typedef TMatrix4<float> Mat4;
typedef TMatrix4<double> DMat4;

const TMatrix4<float> TMatrix4<float>::identity;
const TMatrix4<double> TMatrix4<double>::identity;

typedef Mat4 Transform;

TS_END_PACKAGE1()
