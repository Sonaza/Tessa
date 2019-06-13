
Mat4::Mat4()
{
	// Set matrix to identity
	m_matrix[0] = 1.f; m_matrix[4] = 0.f; m_matrix[8]  = 0.f; m_matrix[12] = 0.f;
	m_matrix[1] = 0.f; m_matrix[5] = 1.f; m_matrix[9]  = 0.f; m_matrix[13] = 0.f;
	m_matrix[2] = 0.f; m_matrix[6] = 0.f; m_matrix[10] = 1.f; m_matrix[14] = 0.f;
	m_matrix[3] = 0.f; m_matrix[7] = 0.f; m_matrix[11] = 0.f; m_matrix[15] = 1.f;
}

Mat4::Mat4(
	float m00, float m01, float m02, float m03,
	float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23,
	float m30, float m31, float m32, float m33)
{
	m_matrix[0] = m00; m_matrix[4] = m01; m_matrix[8]  = m02; m_matrix[12] = m03;
	m_matrix[1] = m10; m_matrix[5] = m11; m_matrix[9]  = m12; m_matrix[13] = m13;
	m_matrix[2] = m20; m_matrix[6] = m21; m_matrix[10] = m22; m_matrix[14] = m23;
	m_matrix[3] = m30; m_matrix[7] = m31; m_matrix[11] = m32; m_matrix[15] = m33;
}

inline const float *Mat4::getMatrix() const
{
	return m_matrix;
}

inline Mat4 Mat4::getInverse() const
{
	// Precalculate some 2x2 determinants
	float s0 = m_matrix[0] * m_matrix[5] - m_matrix[4] * m_matrix[1];
	float s1 = m_matrix[0] * m_matrix[6] - m_matrix[4] * m_matrix[2];
	float s2 = m_matrix[0] * m_matrix[7] - m_matrix[4] * m_matrix[3];
	float s3 = m_matrix[1] * m_matrix[6] - m_matrix[5] * m_matrix[2];
	float s4 = m_matrix[1] * m_matrix[7] - m_matrix[5] * m_matrix[3];
	float s5 = m_matrix[2] * m_matrix[7] - m_matrix[6] * m_matrix[3];

	float c5 = m_matrix[10] * m_matrix[15] - m_matrix[14] * m_matrix[11];
	float c4 = m_matrix[9]  * m_matrix[15] - m_matrix[13] * m_matrix[11];
	float c3 = m_matrix[9]  * m_matrix[14] - m_matrix[13] * m_matrix[10];
	float c2 = m_matrix[8]  * m_matrix[15] - m_matrix[12] * m_matrix[11];
	float c1 = m_matrix[8]  * m_matrix[14] - m_matrix[12] * m_matrix[10];
	float c0 = m_matrix[8]  * m_matrix[13] - m_matrix[12] * m_matrix[9];

	// Calculate determinant for the whole 4x4
	float det = s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;

	// Check determinant to avoid division by zero
	if(det != 0.f)
	{
		float invdet = 1.f / det;

		// Calculate the inverse
		return Mat4(
			( m_matrix[5]  * c5	- m_matrix[6]  * c4	+ m_matrix[7]  * c3) * invdet,
			(-m_matrix[1]  * c5	+ m_matrix[2]  * c4	- m_matrix[3]  * c3) * invdet,
			( m_matrix[13] * s5 - m_matrix[14] * s4 + m_matrix[15] * s3) * invdet,
			(-m_matrix[9]  * s5	+ m_matrix[10] * s4 - m_matrix[11] * s3) * invdet,
			(-m_matrix[4]  * c5	+ m_matrix[6]  * c2	- m_matrix[7]  * c1) * invdet,
			( m_matrix[0]  * c5	- m_matrix[2]  * c2	+ m_matrix[3]  * c1) * invdet,
			(-m_matrix[12] * s5 + m_matrix[14] * s2 - m_matrix[15] * s1) * invdet,
			( m_matrix[8]  * s5	- m_matrix[10] * s2 + m_matrix[11] * s1) * invdet,
			( m_matrix[4]  * c4	- m_matrix[5]  * c2	+ m_matrix[7]  * c0) * invdet,
			(-m_matrix[0]  * c4	+ m_matrix[1]  * c2	- m_matrix[3]  * c0) * invdet,
			( m_matrix[12] * s4 - m_matrix[13] * s2 + m_matrix[15] * s0) * invdet,
			(-m_matrix[8]  * s4	+ m_matrix[9]  * s2	- m_matrix[11] * s0) * invdet,
			(-m_matrix[4]  * c3	+ m_matrix[5]  * c1	- m_matrix[6]  * c0) * invdet,
			( m_matrix[0]  * c3	- m_matrix[1]  * c1	+ m_matrix[2]  * c0) * invdet,
			(-m_matrix[12] * s3 + m_matrix[13] * s1 - m_matrix[14] * s0) * invdet,
			( m_matrix[8]  * s3	- m_matrix[9]  * s1	+ m_matrix[10] * s0) * invdet
		);
	}
	else
	{
		return Mat4();
	}
}

inline Mat4 &Mat4::lookAt(const VC3 &eye, const VC3 &target, const VC3 &up)
{
	VC3 f = normalize(target - eye);
	VC3 u = normalize(up);
	VC3 s = normalize(cross(f, u));
	u = cross(s, f);

	*this = Mat4(
		s.x,	 s.y,	 s.z,	-dot(s, eye),
		u.x,	 u.y,	 u.z,	-dot(u, eye),
		-f.x,	-f.y,	-f.z,	 dot(f, eye),
		0.f,	0.f,	0.f,	1.f
	);

	return *this;
}

inline Mat4 &Mat4::perspective(float fov, float aspect, float znear, float zfar)
{
	float range = std::tan(fov * 0.5f * 3.141592653589f / 180.f);
	float left = -range * aspect;
	float right = range * aspect;
	float bottom = -range;
	float top = range;

	*this = Mat4(
		(2.f * znear) / (right - left),	0.f, 0.f, 0.f,
		0.f, (2.f * znear) / (top - bottom), 0.f, 0.f,
		0.f, 0.f, -(zfar + znear) / (zfar - znear), -(2.f * zfar * znear) / (zfar - znear),
		0.f, 0.f, -1.f,	0.f
	);
	return *this;
}

inline Mat4 &Mat4::ortho(float left, float right, float bottom, float top, float znear, float zfar)
{
	*this = Mat4(
		2.f / (right - left), 0.f, 0.f,	-(right + left) / (right - left),
		0.f, 2.f / (top - bottom), 0.f,	-(top + bottom) / (top - bottom),
		0.f, 0.f, -2.f / (zfar - znear), -(zfar + znear) / (zfar - znear),
		0.f, 0.f, 0.f, 1.f
	);
	return *this;
}

inline Mat4 &Mat4::combine(const Mat4 &matrix)
{
	const float *a = m_matrix;
	const float *b = matrix.m_matrix;

	// Calculate square 4x4 matrix multiplication
	*this = Mat4(
		a[0] * b[0]		+ a[4] * b[1]	+ a[8] * b[2]	+ a[12] * b[3],
		a[0] * b[4]		+ a[4] * b[5]	+ a[8] * b[6]	+ a[12] * b[7],
		a[0] * b[8]		+ a[4] * b[9]	+ a[8] * b[10]	+ a[12] * b[11],
		a[0] * b[12]	+ a[4] * b[13]	+ a[8] * b[14]	+ a[12] * b[15],

		a[1] * b[0]		+ a[5] * b[1]	+ a[9] * b[2]	+ a[13] * b[3],
		a[1] * b[4]		+ a[5] * b[5]	+ a[9] * b[6]	+ a[13] * b[7],
		a[1] * b[8]		+ a[5] * b[9]	+ a[9] * b[10]	+ a[13] * b[11],
		a[1] * b[12]	+ a[5] * b[13]	+ a[9] * b[14]	+ a[13] * b[15],

		a[2] * b[0]		+ a[6] * b[1]	+ a[10] * b[2]	+ a[14] * b[3],
		a[2] * b[4]		+ a[6] * b[5]	+ a[10] * b[6]	+ a[14] * b[7],
		a[2] * b[8]		+ a[6] * b[9]	+ a[10] * b[10]	+ a[14] * b[11],
		a[2] * b[12]	+ a[6] * b[13]	+ a[10] * b[14]	+ a[14] * b[15],

		a[3] * b[0]		+ a[7] * b[1]	+ a[11] * b[2]	+ a[15] * b[3],
		a[3] * b[4]		+ a[7] * b[5]	+ a[11] * b[6]	+ a[15] * b[7],
		a[3] * b[8]		+ a[7] * b[9]	+ a[11] * b[10]	+ a[15] * b[11],
		a[3] * b[12]	+ a[7] * b[13]	+ a[11] * b[14]	+ a[15] * b[15]
	);
	return *this;
}

inline Mat4 &Mat4::translate(const VC3 &a)
{
	Mat4 translation(
		1.f, 0.f, 0.f, a.x,
		0.f, 1.f, 0.f, a.y,
		0.f, 0.f, 1.f, a.z,
		0.f, 0.f, 0.f, 1.f
	);

	return combine(translation);
}

inline Mat4 &Mat4::rotate(float amount, const VC3 &axis)
{
	float rad = amount * 3.141592653589f / 180.f;
	float cos = 1.f - std::cos(rad);
	float sin = std::sin(rad);

	// Normalize axis Vector
	VC3 u = normalize(axis);

	// Rotate matrix around u-axis
	Mat4 rotation(
		(1.f-cos) + u.x * u.x * cos,	u.x * u.y * cos - u.z * sin,	u.x * u.z * cos + u.y * sin,	0.f,
		u.y * u.x * cos + u.z * sin,	(1.f-cos) + u.y * u.y * cos,	u.y * u.z * cos - u.x * sin,	0.f,
		u.z * u.x * cos - u.y * sin,	u.z * u.y * cos + u.x * sin,	(1.f-cos) + u.z * u.z * cos,	0.f,
		0.f,							0.f,							0.f,							1.f
	);

	return combine(rotation);
}

inline Mat4 &Mat4::scale(const VC3 &a)
{
	Mat4 scaling(
		a.x, 0.f, 0.f, 0.f,
		0.f, a.y, 0.f, 0.f,
		0.f, 0.f, a.z, 0.f,
		0.f, 0.f, 0.f, 1.f
	);

	return combine(scaling);
}

inline Mat4 &Mat4::transpose()
{
	*this = Mat4(
		m_matrix[0],  m_matrix[1],  m_matrix[2],  m_matrix[3],
		m_matrix[4],  m_matrix[5],  m_matrix[6],  m_matrix[7],
		m_matrix[8],  m_matrix[9],  m_matrix[10], m_matrix[11],
		m_matrix[12], m_matrix[13], m_matrix[14], m_matrix[15]
	);

	return *this;
}

inline Mat4 operator*(const Mat4 &lhs, const Mat4 &rhs)
{
	return Mat4(lhs).combine(rhs);
}

inline VC4 operator*(const Mat4 &mat, const VC4& v)
{
	const float *a = mat.getMatrix();

	return VC4(
		v.x * a[0] + v.y * a[4] + v.z * a[8]  + v.w * a[12],
		v.x * a[1] + v.y * a[5] + v.z * a[9]  + v.w * a[13],
		v.x * a[2] + v.y * a[6] + v.z * a[10] + v.w * a[14],
		v.x * a[3] + v.y * a[7] + v.z * a[11] + v.w * a[15]
	);
}
