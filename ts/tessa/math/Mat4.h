#pragma once

#include "ts/tessa/math/Vec2.h"
#include "ts/tessa/math/Vec3.h"
#include "ts/tessa/math/Vec4.h"

#if defined(SFML_VERSION_MAJOR)
#include "SFML/Graphics/Transform.hpp"
#endif

#include <cmath>

TS_PACKAGE1(math)

class Mat4
{
public:
	static const Mat4 identity;

	// Constructs an identity matrix
	Mat4();

	// Constructs from 3x3 matrix by expanding to 4x4
	Mat4(float m00, float m01, float m02,
		 float m10, float m11, float m12,
		 float m20, float m21, float m22);

	// Construct directly from individual components
	Mat4(float m00, float m01, float m02, float m03,
		 float m10, float m11, float m12, float m13,
		 float m20, float m21, float m22, float m23,
		 float m30, float m31, float m32, float m33);

	// Default copy operations
	Mat4(const Mat4 &other) = default;
	Mat4 &operator=(const Mat4 &other) = default;

	// Retrieve current matrix
	const float *getMatrix() const;

	// Calculate inverse matrix
	Mat4 getInverse() const;

	// Matrix multiplication
	Mat4 &combine(const Mat4 &matrix);

	// Translate matrix by offset
	Mat4 &translate(const VC2 &offset);
	Mat4 &translate(float x, float y);
	Mat4 &translate(const VC3 &offset);

	// Rotates matrix by given amount on Z axis (for 2D graphics)
	Mat4 &rotate(float degrees, const VC2 &center = VC2::zero);

	// Rotate matrix by amount on specified axis
	Mat4 &rotate(float degrees, const VC3 &axis);

	// Scales matrix by amount
	Mat4 &scale(const VC2 &scale, const VC2 &center = VC2::zero);
	Mat4 &scale(float scaleX, float scaleY, float centerX = 0.f, float centerY = 0.f);
	Mat4 &scale(const VC3 &scale);

	// Calculate matrix transpose
	Mat4 &transpose();

	// Transforms a VC2 point by the current matrix
	VC2 transformPoint(const VC2 &point) const;

	// Transforms a VC3 point by the current matrix
	VC3 transformPoint(const VC3 &point) const;

	// Transforms a VC4 point by the current matrix
	VC4 transformPoint(const VC4 &point) const;

	// Calculate a lookat matrix 
	static Mat4 makeLookAt(const VC3 &eye, const VC3 &target, const VC3 &up);

	// Calculates 3D projection matrix
	static Mat4 makePerspective(float fovInDegrees, float aspect, float znear, float zfar);

	// Calculates orthographic projection matrix
	static Mat4 makeOrtho(float left, float right,
		float bottom, float top, float znear = -1.f, float zfar = 1.f);

#if defined(SFML_VERSION_MAJOR)
	Mat4(const sf::Transform &transform);
	explicit operator sf::Transform() const;
#endif

	// Row utility class for bracket access
	class Row
	{
	public:
		Row(float a, float b, float c, float d)
		{
			m_row[0] = a;
			m_row[1] = b;
			m_row[2] = c;
			m_row[3] = d;
		}

		float operator[](unsigned int i) const
		{
			TS_ASSERT(i >= 0 && i < 4);
			return m_row[i];
		}

	private:
		float m_row[4];
	};

	// Overload brackets to retrieve a single row
	const Row operator[](unsigned int i)
	{
		TS_ASSERT(i >= 0 && i < 4);
		return Row(
			m_matrix[i * 4],
			m_matrix[i * 4 + 1],
			m_matrix[i * 4 + 2],
			m_matrix[i * 4 + 3]
		);
	}

private:
	float m_matrix[16];
};

bool operator==(const Mat4 &lhs, const Mat4 &rhs);
bool operator!=(const Mat4 &lhs, const Mat4 &rhs);

Mat4 operator*(const Mat4 &lhs, const Mat4 &rhs);

// Transform Vector by matrix
VC2 operator*(const Mat4 &mat, const VC2 &point);
VC3 operator*(const Mat4 &mat, const VC3 &point);
VC4 operator*(const Mat4 &mat, const VC4 &point);

typedef Mat4 Transform;

TS_END_PACKAGE1()
