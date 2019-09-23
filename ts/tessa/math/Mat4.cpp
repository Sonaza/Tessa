#include "Precompiled.h"
#include "Mat4.h"

#include "ts/tessa/math/Angle.h"

TS_PACKAGE1(math)

const Mat4 Mat4::identity;

Mat4::Mat4()
{
	// Set matrix to identity
	m_matrix[0] = 1.f; m_matrix[4] = 0.f; m_matrix[8]  = 0.f; m_matrix[12] = 0.f;
	m_matrix[1] = 0.f; m_matrix[5] = 1.f; m_matrix[9]  = 0.f; m_matrix[13] = 0.f;
	m_matrix[2] = 0.f; m_matrix[6] = 0.f; m_matrix[10] = 1.f; m_matrix[14] = 0.f;
	m_matrix[3] = 0.f; m_matrix[7] = 0.f; m_matrix[11] = 0.f; m_matrix[15] = 1.f;
}

Mat4::Mat4(
	float m00, float m01, float m02,
	float m10, float m11, float m12,
	float m20, float m21, float m22)
{
	// Constructs from 3x3 matrix by expanding to 4x4
	m_matrix[0] = m00; m_matrix[4] = m01; m_matrix[8]  = 0.f; m_matrix[12] = m02;
	m_matrix[1] = m10; m_matrix[5] = m11; m_matrix[9]  = 0.f; m_matrix[13] = m12;
	m_matrix[2] = 0.f; m_matrix[6] = 0.f; m_matrix[10] = 1.f; m_matrix[14] = 0.f;
	m_matrix[3] = m20; m_matrix[7] = m21; m_matrix[11] = 0.f; m_matrix[15] = m22;
}

Mat4::Mat4(
	float m00, float m01, float m02, float m03,
	float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23,
	float m30, float m31, float m32, float m33)
{
	// Construct directly from individual components
	m_matrix[0] = m00; m_matrix[4] = m01; m_matrix[8]  = m02; m_matrix[12] = m03;
	m_matrix[1] = m10; m_matrix[5] = m11; m_matrix[9]  = m12; m_matrix[13] = m13;
	m_matrix[2] = m20; m_matrix[6] = m21; m_matrix[10] = m22; m_matrix[14] = m23;
	m_matrix[3] = m30; m_matrix[7] = m31; m_matrix[11] = m32; m_matrix[15] = m33;
}

const float *Mat4::getMatrix() const
{
	return m_matrix;
}

Mat4 Mat4::getInverse() const
{
	float A2323 = m_matrix[10] * m_matrix[15] - m_matrix[14] * m_matrix[11];
	float A1223 = m_matrix[6]  * m_matrix[11] - m_matrix[10] * m_matrix[7];
	float A1323 = m_matrix[6]  * m_matrix[15] - m_matrix[14] * m_matrix[7];
	float A0323 = m_matrix[2]  * m_matrix[15] - m_matrix[14] * m_matrix[3];
	float A0223 = m_matrix[2]  * m_matrix[11] - m_matrix[10] * m_matrix[3];
	float A0123 = m_matrix[2]  * m_matrix[7]  - m_matrix[6]  * m_matrix[3];
	float A2313 = m_matrix[9]  * m_matrix[15] - m_matrix[13] * m_matrix[11];
	float A1313 = m_matrix[5]  * m_matrix[15] - m_matrix[13] * m_matrix[7];
	float A1213 = m_matrix[5]  * m_matrix[11] - m_matrix[9]  * m_matrix[7];
	float A2312 = m_matrix[9]  * m_matrix[14] - m_matrix[13] * m_matrix[10];
	float A1312 = m_matrix[5]  * m_matrix[14] - m_matrix[13] * m_matrix[6];
	float A1212 = m_matrix[5]  * m_matrix[10] - m_matrix[9]  * m_matrix[6];
	float A0313 = m_matrix[1]  * m_matrix[15] - m_matrix[13] * m_matrix[3];
	float A0213 = m_matrix[1]  * m_matrix[11] - m_matrix[9]  * m_matrix[3];
	float A0312 = m_matrix[1]  * m_matrix[14] - m_matrix[13] * m_matrix[2];
	float A0212 = m_matrix[1]  * m_matrix[10] - m_matrix[9]  * m_matrix[2];
	float A0113 = m_matrix[1]  * m_matrix[7]  - m_matrix[5]  * m_matrix[3];
	float A0112 = m_matrix[1]  * m_matrix[6]  - m_matrix[5]  * m_matrix[2];

	float det = m_matrix[0]  * (m_matrix[5] * A2323 - m_matrix[9] * A1323 + m_matrix[13] * A1223)
	          - m_matrix[4]  * (m_matrix[1] * A2323 - m_matrix[9] * A0323 + m_matrix[13] * A0223)
	          + m_matrix[8]  * (m_matrix[1] * A1323 - m_matrix[5] * A0323 + m_matrix[13] * A0123)
	          - m_matrix[12] * (m_matrix[1] * A1223 - m_matrix[5] * A0223 + m_matrix[9]  * A0123);

	// Check determinant to avoid division by zero
	if (det != 0.f)
	{
		float invdet = 1.f / det;
		return Mat4(
			 (m_matrix[5] * A2323 - m_matrix[9] * A1323 + m_matrix[13] * A1223) * invdet,
			-(m_matrix[4] * A2323 - m_matrix[8] * A1323 + m_matrix[12] * A1223) * invdet,
			 (m_matrix[4] * A2313 - m_matrix[8] * A1313 + m_matrix[12] * A1213) * invdet,
			-(m_matrix[4] * A2312 - m_matrix[8] * A1312 + m_matrix[12] * A1212) * invdet,

			-(m_matrix[1] * A2323 - m_matrix[9] * A0323 + m_matrix[13] * A0223) * invdet,
			 (m_matrix[0] * A2323 - m_matrix[8] * A0323 + m_matrix[12] * A0223) * invdet,
			-(m_matrix[0] * A2313 - m_matrix[8] * A0313 + m_matrix[12] * A0213) * invdet,
			 (m_matrix[0] * A2312 - m_matrix[8] * A0312 + m_matrix[12] * A0212) * invdet,

			 (m_matrix[1] * A1323 - m_matrix[5] * A0323 + m_matrix[13] * A0123) * invdet,
			-(m_matrix[0] * A1323 - m_matrix[4] * A0323 + m_matrix[12] * A0123) * invdet,
			 (m_matrix[0] * A1313 - m_matrix[4] * A0313 + m_matrix[12] * A0113) * invdet,
			-(m_matrix[0] * A1312 - m_matrix[4] * A0312 + m_matrix[12] * A0112) * invdet,

			-(m_matrix[1] * A1223 - m_matrix[5] * A0223 + m_matrix[9]  * A0123) * invdet,
			 (m_matrix[0] * A1223 - m_matrix[4] * A0223 + m_matrix[8]  * A0123) * invdet,
			-(m_matrix[0] * A1213 - m_matrix[4] * A0213 + m_matrix[8]  * A0113) * invdet,
			 (m_matrix[0] * A1212 - m_matrix[4] * A0212 + m_matrix[8]  * A0112) * invdet
		);
	}
	else
	{
		return Mat4::identity;
	}
}

Mat4 &Mat4::combine(const Mat4 &matrix)
{
	const float *a = m_matrix;
	const float *b = matrix.m_matrix;

	// Calculate square 4x4 matrix multiplication
	*this = Mat4(
		a[0] * b[0]     + a[4] * b[1]   + a[8] * b[2]   + a[12] * b[3],
		a[0] * b[4]     + a[4] * b[5]   + a[8] * b[6]   + a[12] * b[7],
		a[0] * b[8]     + a[4] * b[9]   + a[8] * b[10]  + a[12] * b[11],
		a[0] * b[12]    + a[4] * b[13]  + a[8] * b[14]  + a[12] * b[15],

		a[1] * b[0]     + a[5] * b[1]   + a[9] * b[2]   + a[13] * b[3],
		a[1] * b[4]     + a[5] * b[5]   + a[9] * b[6]   + a[13] * b[7],
		a[1] * b[8]     + a[5] * b[9]   + a[9] * b[10]  + a[13] * b[11],
		a[1] * b[12]    + a[5] * b[13]  + a[9] * b[14]  + a[13] * b[15],

		a[2] * b[0]     + a[6] * b[1]   + a[10] * b[2]  + a[14] * b[3],
		a[2] * b[4]     + a[6] * b[5]   + a[10] * b[6]  + a[14] * b[7],
		a[2] * b[8]     + a[6] * b[9]   + a[10] * b[10] + a[14] * b[11],
		a[2] * b[12]    + a[6] * b[13]  + a[10] * b[14] + a[14] * b[15],

		a[3] * b[0]     + a[7] * b[1]   + a[11] * b[2]  + a[15] * b[3],
		a[3] * b[4]     + a[7] * b[5]   + a[11] * b[6]  + a[15] * b[7],
		a[3] * b[8]     + a[7] * b[9]   + a[11] * b[10] + a[15] * b[11],
		a[3] * b[12]    + a[7] * b[13]  + a[11] * b[14] + a[15] * b[15]
	);
	return *this;
}

Mat4 &Mat4::translate(const VC2 &v)
{
	return translate(v.x, v.y);
}

Mat4 &Mat4::translate(float x, float y)
{
	Mat4 translation(
		1.f, 0.f, 0.f, x,
		0.f, 1.f, 0.f, y,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f
	);
	return combine(translation);
}

Mat4 &Mat4::translate(const VC3 &v)
{
	Mat4 translation(
		1.f, 0.f, 0.f, v.x,
		0.f, 1.f, 0.f, v.y,
		0.f, 0.f, 1.f, v.z,
		0.f, 0.f, 0.f, 1.f
	);
	return combine(translation);
}

Mat4 &Mat4::rotate(float degrees, const math::VC2 &center)
{
	float rad = degToRad(degrees);
	float cos = std::cos(rad);
	float sin = std::sin(rad);

	Mat4 rotation(
		cos, -sin, 0.f, center.x * (1 - cos) + center.y * sin,
		sin,  cos, 0.f, center.y * (1 - cos) - center.x * sin,
		0.f,  0.f, 1.f, 0.f,
		0.f,  0.f, 0.f, 1.f
	);

	return combine(rotation);
}

Mat4 &Mat4::rotate(float degrees, const VC3 &axis)
{
	float rad = degToRad(degrees);
	float cos = 1.f - std::cos(rad);
	float sin = std::sin(rad);

	// Normalize axis Vector
	TS_ASSERT(axis.squarelength() > 0.f);
	VC3 u = normalize(axis);

	// Rotate matrix around u-axis
	Mat4 rotation(
		(1.f-cos) + u.x * u.x * cos,  u.x * u.y * cos - u.z * sin,  u.x * u.z * cos + u.y * sin,  0.f,
		u.y * u.x * cos + u.z * sin,  (1.f-cos) + u.y * u.y * cos,  u.y * u.z * cos - u.x * sin,  0.f,
		u.z * u.x * cos - u.y * sin,  u.z * u.y * cos + u.x * sin,  (1.f-cos) + u.z * u.z * cos,  0.f,
		0.f,                          0.f,                          0.f,                          1.f
	);

	return combine(rotation);
}

Mat4 &Mat4::scale(const VC2 &s, const VC2 &c)
{
	return scale(s.x, s.y, c.x, c.y);
}

Mat4 &Mat4::scale(float scaleX, float scaleY, float centerX, float centerY)
{
	Mat4 scaling(
		scaleX, 0.f,    0.f, centerX * (1 - scaleX),
		0.f,    scaleX, 0.f, centerY * (1 - scaleY),
		0.f,    0.f,    1.f, 0.f,
		0.f,    0.f,    0.f, 1.f
	);
	return combine(scaling);
}

Mat4 &Mat4::scale(const VC3 &s)
{
	Mat4 scaling(
		s.x, 0.f, 0.f, 0.f,
		0.f, s.y, 0.f, 0.f,
		0.f, 0.f, s.z, 0.f,
		0.f, 0.f, 0.f, 1.f
	);
	return combine(scaling);
}

Mat4 &Mat4::transpose()
{
	*this = Mat4(
		m_matrix[0],  m_matrix[1],  m_matrix[2],  m_matrix[3],
		m_matrix[4],  m_matrix[5],  m_matrix[6],  m_matrix[7],
		m_matrix[8],  m_matrix[9],  m_matrix[10], m_matrix[11],
		m_matrix[12], m_matrix[13], m_matrix[14], m_matrix[15]
	);
	return *this;
}

VC2 Mat4::transformPoint(const VC2 &point) const
{
	return VC2(
		point.x * m_matrix[0] + point.y * m_matrix[4] + m_matrix[12],
		point.x * m_matrix[1] + point.y * m_matrix[5] + m_matrix[13]
	);
};

VC3 Mat4::transformPoint(const VC3 &point) const
{
	return VC3(
		point.x * m_matrix[0] + point.y * m_matrix[4] + point.z * m_matrix[8]  + m_matrix[12],
		point.x * m_matrix[1] + point.y * m_matrix[5] + point.z * m_matrix[9]  + m_matrix[13],
		point.x * m_matrix[2] + point.y * m_matrix[6] + point.z * m_matrix[10] + m_matrix[14]
	);
};

VC4 Mat4::transformPoint(const VC4 &point) const
{
	return VC4(
		point.x * m_matrix[0] + point.y * m_matrix[4] + point.z * m_matrix[8]  + point.w * m_matrix[12],
		point.x * m_matrix[1] + point.y * m_matrix[5] + point.z * m_matrix[9]  + point.w * m_matrix[13],
		point.x * m_matrix[2] + point.y * m_matrix[6] + point.z * m_matrix[10] + point.w * m_matrix[14],
		point.x * m_matrix[3] + point.y * m_matrix[7] + point.z * m_matrix[11] + point.w * m_matrix[15]
	);
};

Mat4 Mat4::makeLookAt(const VC3 &eye, const VC3 &target, const VC3 &up)
{
	VC3 f = normalize(target - eye);
	VC3 u = normalize(up);
	VC3 s = normalize(cross(f, u));
	u = cross(s, f);

	return Mat4(
		 s.x,  s.y,  s.z, -dot(s, eye),
		 u.x,  u.y,  u.z, -dot(u, eye),
		-f.x, -f.y, -f.z,  dot(f, eye),
		 0.f,  0.f,  0.f,  1.f
	);
}

Mat4 Mat4::makePerspective(float fovInDegrees, float aspect, float znear, float zfar)
{
	float range = std::tan(degToRad(fovInDegrees) * 0.5f);
	float left = -range * aspect;
	float right = range * aspect;
	float bottom = -range;
	float top = range;

	return Mat4(
		(2.f * znear) / (right - left),	0.f, 0.f, 0.f,
		0.f, (2.f * znear) / (top - bottom), 0.f, 0.f,
		0.f, 0.f, -(zfar + znear) / (zfar - znear), -(2.f * zfar * znear) / (zfar - znear),
		0.f, 0.f, -1.f,	0.f
	);
}

Mat4 Mat4::makeOrtho(float left, float right, float bottom, float top, float znear, float zfar)
{
	return Mat4(
		2.f / (right - left), 0.f, 0.f,	-(right + left) / (right - left),
		0.f, 2.f / (top - bottom), 0.f,	-(top + bottom) / (top - bottom),
		0.f, 0.f, -2.f / (zfar - znear), -(zfar + znear) / (zfar - znear),
		0.f, 0.f, 0.f, 1.f
	);
}

#if TS_GLOBAL_USING_SFML == TS_TRUE

Mat4::Mat4(const sf::Transform &transform)
{
	const float *m = transform.getMatrix();
	*this = Mat4(
		m[0], m[4], m[8],  m[12],
		m[1], m[5], m[9],  m[13],
		m[2], m[6], m[10], m[14],
		m[3], m[7], m[11], m[15]
	);
}

Mat4::operator sf::Transform() const
{
	// Basically convert from 4x4 to 3x3 (even though SFML will then re-expand to 4x4)
	return sf::Transform(
		m_matrix[0], m_matrix[4], m_matrix[12],
		m_matrix[1], m_matrix[5], m_matrix[13],
		m_matrix[3], m_matrix[7], m_matrix[15]
	);
}

#endif

bool operator==(const Mat4 &lhs, const Mat4 &rhs)
{
	const float *lm = lhs.getMatrix();
	const float *rm = rhs.getMatrix();
	for (int32_t i = 0; i < 16; ++i)
	{
		if (lm[i] != rm[i])
			return false;
	}
	return true;
}

bool operator!=(const Mat4 &lhs, const Mat4 &rhs)
{
	return !(lhs == rhs);
}

Mat4 operator*(const Mat4 &lhs, const Mat4 &rhs)
{
	return Mat4(lhs).combine(rhs);
}

VC2 operator*(const Mat4 &mat, const VC2 &point)
{
	return mat.transformPoint(point);
}

VC3 operator*(const Mat4 &mat, const VC3 &point)
{
	return mat.transformPoint(point);
}

VC4 operator*(const Mat4 &mat, const VC4 &point)
{
	return mat.transformPoint(point);
}

TS_END_PACKAGE1()
