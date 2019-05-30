#pragma once

#include "ts/tessa/math/Vec3.h"
#include "ts/tessa/math/Vec4.h"

TS_PACKAGE1(math)

class Mat4
{
public:
	Mat4();
	Mat4(float m00, float m01, float m02, float m03,
		 float m10, float m11, float m12, float m13,
		 float m20, float m21, float m22, float m23,
		 float m30, float m31, float m32, float m33);

	// Retrieve current matrix
	const float* getMatrix() const;

	// Calculate inverse matrix
	Mat4 getInverse() const;

	// Calculate a lookat matrix 
	Mat4& lookAt(const Vec3f& eye, const Vec3f& target, const Vec3f& up);

	// Calculates 3D projection matrix
	Mat4& perspective(float fov, float aspect, float znear, float zfar);

	// Calculates orthographic projection matrix
	Mat4& ortho(float left, float right, float bottom, float top, float znear = -1.f, float zfar = 1.f);

	// Matrix multiplication
	Mat4& combine(const Mat4& matrix);

	// Translate matrix by offset
	Mat4& translate(const Vec3f& offset);

	// Rotate matrix by amount on specified axis
	Mat4& rotate(float amount, const Vec3f& axis);

	// Scale matrix by amount
	Mat4& scale(const Vec3f &amount);

	// Calculate matrix transpose
	Mat4& transpose();

	// Row utility class for bracket access
	class row
	{
	public:
		row(float a, float b, float c, float d)
		{
			m_row[0] = a;
			m_row[1] = b;
			m_row[2] = c;
			m_row[3] = d;
		}

		float operator[](unsigned int i) const { TS_ASSERT(i >= 0 && i < 4); return m_row[i]; }

	private:
		float m_row[4];
	};

	// Overload brackets to retrieve a single row
	const row operator[](unsigned int i) { TS_ASSERT(i >= 0 && i < 4); return row(m_matrix[i*4], m_matrix[i*4+1], m_matrix[i*4+2], m_matrix[i*4+3]); }

private:

	float m_matrix[16];

};

Mat4 operator*(const Mat4& lhs, const Mat4& rhs);

// Transform Vector by matrix
Vec4f operator*(const Mat4& mat, const Vec4f& v);

#include "Mat4.inl"

TS_END_PACKAGE1()
