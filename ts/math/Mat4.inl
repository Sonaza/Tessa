
template<class T>
TMatrix4<T>::TMatrix4()
{
	// Set matrix to identity
	m_matrix[0] = 1; m_matrix[4] = 0; m_matrix[8]  = 0; m_matrix[12] = 0;
	m_matrix[1] = 0; m_matrix[5] = 1; m_matrix[9]  = 0; m_matrix[13] = 0;
	m_matrix[2] = 0; m_matrix[6] = 0; m_matrix[10] = 1; m_matrix[14] = 0;
	m_matrix[3] = 0; m_matrix[7] = 0; m_matrix[11] = 0; m_matrix[15] = 1;
}

template<class T>
TMatrix4<T>::TMatrix4(
	T m00, T m01, T m02,
	T m10, T m11, T m12,
	T m20, T m21, T m22)
{
	// Constructs from 3x3 matrix by expanding to 4x4
	m_matrix[0] = m00; m_matrix[4] = m01; m_matrix[8]  = 0; m_matrix[12] = m02;
	m_matrix[1] = m10; m_matrix[5] = m11; m_matrix[9]  = 0; m_matrix[13] = m12;
	m_matrix[2] = 0;   m_matrix[6] = 0;   m_matrix[10] = 1; m_matrix[14] = 0;
	m_matrix[3] = m20; m_matrix[7] = m21; m_matrix[11] = 0; m_matrix[15] = m22;
}

template<class T>
TMatrix4<T>::TMatrix4(
	T m00, T m01, T m02, T m03,
	T m10, T m11, T m12, T m13,
	T m20, T m21, T m22, T m23,
	T m30, T m31, T m32, T m33)
{
	// Construct directly from individual components
	m_matrix[0] = m00; m_matrix[4] = m01; m_matrix[8]  = m02; m_matrix[12] = m03;
	m_matrix[1] = m10; m_matrix[5] = m11; m_matrix[9]  = m12; m_matrix[13] = m13;
	m_matrix[2] = m20; m_matrix[6] = m21; m_matrix[10] = m22; m_matrix[14] = m23;
	m_matrix[3] = m30; m_matrix[7] = m31; m_matrix[11] = m32; m_matrix[15] = m33;
}

template<class T>
TMatrix4<T>::TMatrix4(T m[16])
{
	memcpy(m_matrix, m, sizeof(T) * 16);
}

template<class T>
inline const T *TMatrix4<T>::getMatrix() const
{
	return m_matrix;
}

template<class T>
inline TMatrix4<T> &TMatrix4<T>::invert()
{
	T det = m_matrix[0] * (m_matrix[5] * m_matrix[10] - m_matrix[9] * m_matrix[6]) -
	        m_matrix[4] * (m_matrix[1] * m_matrix[10] - m_matrix[9] * m_matrix[2]) +
	        m_matrix[8] * (m_matrix[1] * m_matrix[6]  - m_matrix[5] * m_matrix[2]);

	// Check determinant to avoid division by zero
	if (det != 0)
	{
		T invdet = 1 / det;
		T invmat[16] = {};
		
		invmat[0]  =  (m_matrix[5] * m_matrix[10] - m_matrix[9] * m_matrix[6]) * invdet;
		invmat[1]  = -(m_matrix[1] * m_matrix[10] - m_matrix[9] * m_matrix[2]) * invdet;
		invmat[2]  =  (m_matrix[1] * m_matrix[6] - m_matrix[5] * m_matrix[2]) * invdet;
		invmat[3]  =  0;
		
		invmat[4]  = -(m_matrix[4] * m_matrix[10] - m_matrix[8] * m_matrix[6]) * invdet;
		invmat[5]  =  (m_matrix[0] * m_matrix[10] - m_matrix[8] * m_matrix[2]) * invdet;
		invmat[6]  = -(m_matrix[0] * m_matrix[6] - m_matrix[4] * m_matrix[2]) * invdet;
		invmat[7]  =  0;
		
		invmat[8]  =  (m_matrix[4] * m_matrix[9] - m_matrix[8] * m_matrix[5]) * invdet;
		invmat[9]  = -(m_matrix[0] * m_matrix[9] - m_matrix[8] * m_matrix[1]) * invdet;
		invmat[10] =  (m_matrix[0] * m_matrix[5] - m_matrix[4] * m_matrix[1]) * invdet;
		invmat[11] =  0;

		invmat[12] = -(m_matrix[12] * invmat[0] + m_matrix[13] * invmat[4] + m_matrix[14] * invmat[8]);
		invmat[13] = -(m_matrix[12] * invmat[1] + m_matrix[13] * invmat[5] + m_matrix[14] * invmat[9]);
		invmat[14] = -(m_matrix[12] * invmat[2] + m_matrix[13] * invmat[6] + m_matrix[14] * invmat[10]);
		invmat[15] =  1;
		
		// T A2312 = m_matrix[9] * m_matrix[14] - m_matrix[13] * m_matrix[10];
		// T A1312 = m_matrix[5] * m_matrix[14] - m_matrix[13] * m_matrix[6];
		// T A1212 = m_matrix[5] * m_matrix[10] - m_matrix[9]  * m_matrix[6];
		// T A0312 = m_matrix[1] * m_matrix[14] - m_matrix[13] * m_matrix[2];
		// T A0212 = m_matrix[1] * m_matrix[10] - m_matrix[9]  * m_matrix[2];
		// T A0112 = m_matrix[1] * m_matrix[6]  - m_matrix[5]  * m_matrix[2];
		// invmat[12] = -(m_matrix[4] * A2312 - m_matrix[8] * A1312 + m_matrix[12] * A1212) * invdet;
		// invmat[13] =  (m_matrix[0] * A2312 - m_matrix[8] * A0312 + m_matrix[12] * A0212) * invdet;
		// invmat[14] = -(m_matrix[0] * A1312 - m_matrix[4] * A0312 + m_matrix[12] * A0112) * invdet;

		*this = TMatrix4<T>(invmat);
	}
	else
	{
		*this = TMatrix4<T>::identity;
	}
	
	return *this;
}

template<class T>
inline TMatrix4<T> TMatrix4<T>::getInverse() const
{
	return TMatrix4<T>(*this).invert();
}

template<class T>
inline TMatrix4<T> &TMatrix4<T>::combine(const TMatrix4<T> &matrix)
{
	const T *a = m_matrix;
	const T *b = matrix.m_matrix;

	// Calculate square 4x4 matrix multiplication
	*this = TMatrix4<T>(
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

template<class T>
inline TMatrix4<T> &TMatrix4<T>::translate(const Vec2<T> &v)
{
	return translate(v.x, v.y);
}

template<class T>
inline TMatrix4<T> &TMatrix4<T>::translate(T x, T y)
{
	TMatrix4<T> translation(
		1, 0, 0, x,
		0, 1, 0, y,
		0, 0, 1, 0,
		0, 0, 0, 1
	);
	return combine(translation);
}

template<class T>
inline TMatrix4<T> &TMatrix4<T>::translate(const Vec3<T> &v)
{
	TMatrix4<T> translation(
		1, 0, 0, v.x,
		0, 1, 0, v.y,
		0, 0, 1, v.z,
		0, 0, 0, 1
	);
	return combine(translation);
}

template<class T>
inline TMatrix4<T> &TMatrix4<T>::rotate(T degrees, const Vec2<T> &center)
{
	T rad = (T)degToRad(degrees);
	T cos = (T)std::cos(rad);
	T sin = (T)std::sin(rad);

	TMatrix4<T> rotation(
		cos, -sin, 0, center.x * (1 - cos) + center.y * sin,
		sin,  cos, 0, center.y * (1 - cos) - center.x * sin,
		0,    0,   1, 0,
		0,    0,   0, 1
	);

	return combine(rotation);
}

template<class T>
inline TMatrix4<T> &TMatrix4<T>::rotate(T degrees, const Vec3<T> &axis)
{
	T rad = (T)degToRad((T)degrees);
	T cos = (T)(1.0 - std::cos(rad));
	T sin = (T)std::sin(rad);

	// Normalize axis Vector
	Vec3<T> u = axis.getNormalized();

	// Rotate matrix around u-axis
	TMatrix4<T> rotation(
		(1 - cos) + u.x * u.x * cos,  u.x * u.y * cos - u.z * sin,  u.x * u.z * cos + u.y * sin,  0,
		u.y * u.x * cos + u.z * sin,  (1 - cos) + u.y * u.y * cos,  u.y * u.z * cos - u.x * sin,  0,
		u.z * u.x * cos - u.y * sin,  u.z * u.y * cos + u.x * sin,  (1 - cos) + u.z * u.z * cos,  0,
		0,                            0,                            0,                            1
	);

	return combine(rotation);
}

template<class T>
inline TMatrix4<T> &TMatrix4<T>::scale(const Vec2<T> &s, const Vec2<T> &c)
{
	return scale(s.x, s.y, c.x, c.y);
}

template<class T>
inline TMatrix4<T> &TMatrix4<T>::scale(T scaleX, T scaleY, T centerX, T centerY)
{
	TMatrix4<T> scaling(
		scaleX, 0,      0, centerX * (1 - scaleX),
		0,      scaleX, 0, centerY * (1 - scaleY),
		0,      0,      1, 0,
		0,      0,      0, 1
	);
	return combine(scaling);
}

template<class T>
inline TMatrix4<T> &TMatrix4<T>::scale(const Vec3<T> &s)
{
	TMatrix4<T> scaling(
		s.x, 0,   0,   0,
		0,   s.y, 0,   0,
		0,   0,   s.z, 0,
		0,   0,   0,   1
	);
	return combine(scaling);
}

template<class T>
inline TMatrix4<T> &TMatrix4<T>::transpose()
{
	*this = TMatrix4<T>(
		m_matrix[0],  m_matrix[1],  m_matrix[2],  m_matrix[3],
		m_matrix[4],  m_matrix[5],  m_matrix[6],  m_matrix[7],
		m_matrix[8],  m_matrix[9],  m_matrix[10], m_matrix[11],
		m_matrix[12], m_matrix[13], m_matrix[14], m_matrix[15]
	);
	return *this;
}

template<class T>
inline Vec2<T> TMatrix4<T>::transformPoint(const Vec2<T> &point) const
{
	return Vec2<T>(
		point.x * m_matrix[0] + point.y * m_matrix[4] + m_matrix[12],
		point.x * m_matrix[1] + point.y * m_matrix[5] + m_matrix[13]
	);
}

template<class T>
inline Vec3<T> TMatrix4<T>::transformPoint(const Vec3<T> &point) const
{
	return Vec3<T>(
		point.x * m_matrix[0] + point.y * m_matrix[4] + point.z * m_matrix[8]  + m_matrix[12],
		point.x * m_matrix[1] + point.y * m_matrix[5] + point.z * m_matrix[9]  + m_matrix[13],
		point.x * m_matrix[2] + point.y * m_matrix[6] + point.z * m_matrix[10] + m_matrix[14]
	);
}

template<class T>
inline Vec4<T> TMatrix4<T>::transformPoint(const Vec4<T> &point) const
{
	return Vec4<T>(
		point.x * m_matrix[0] + point.y * m_matrix[4] + point.z * m_matrix[8]  + point.w * m_matrix[12],
		point.x * m_matrix[1] + point.y * m_matrix[5] + point.z * m_matrix[9]  + point.w * m_matrix[13],
		point.x * m_matrix[2] + point.y * m_matrix[6] + point.z * m_matrix[10] + point.w * m_matrix[14],
		point.x * m_matrix[3] + point.y * m_matrix[7] + point.z * m_matrix[11] + point.w * m_matrix[15]
	);
}

template<class T>
inline TMatrix4<T> TMatrix4<T>::makeLookAt(const Vec3<T> &eye, const Vec3<T> &target, const Vec3<T> &up)
{
	Vec3<T> f = (target - eye).getNormalized();
	Vec3<T> u = up.getNormalized();
	Vec3<T> s = f.cross(u).getNormalized();
	u = s.cross(f);

	return TMatrix4<T>(
		 s.x,  s.y,  s.z, -s.dot(eye),
		 u.x,  u.y,  u.z, -u.dot(eye),
		-f.x, -f.y, -f.z,  f.dot(eye),
		 0,    0,    0,    1
	);
}

template<class T>
inline TMatrix4<T> TMatrix4<T>::makePerspective(T fovInDegrees, T aspect, T znear, T zfar)
{
	T range = std::tan(degToRad(fovInDegrees) * 0.5f);
	T left = -range * aspect;
	T right = range * aspect;
	T bottom = -range;
	T top = range;

	return TMatrix4<T>(
		(2 * znear) / (right - left), 0, 0, 0,
		0, (2 * znear) / (top - bottom), 0, 0,
		0, 0, -(zfar + znear) / (zfar - znear), -(2 * zfar * znear) / (zfar - znear),
		0, 0, -1,	0
	);
}

template<class T>
inline TMatrix4<T> TMatrix4<T>::makeOrtho(T left, T right, T bottom, T top, T znear, T zfar)
{
	return TMatrix4<T>(
		2 / (right - left), 0, 0, -(right + left) / (right - left),
		0, 2 / (top - bottom), 0, -(top + bottom) / (top - bottom),
		0, 0, -2 / (zfar - znear), -(zfar + znear) / (zfar - znear),
		0, 0, 0, 1
	);
}

#if TS_GLOBAL_USING_SFML == TS_TRUE

template<class T>
inline TMatrix4<T>::TMatrix4(const sf::Transform &transform)
{
	const float *m = transform.getMatrix();
	*this = TMatrix4<T>(
		m[0], m[4], m[8],  m[12],
		m[1], m[5], m[9],  m[13],
		m[2], m[6], m[10], m[14],
		m[3], m[7], m[11], m[15]
	);
}

template<class T>
inline TMatrix4<T>::operator sf::Transform() const
{
	// Basically convert from 4x4 to 3x3 (even though SFML will then re-expand to 4x4)
	return sf::Transform(
		float(m_matrix[0]), float(m_matrix[4]), float(m_matrix[12]),
		float(m_matrix[1]), float(m_matrix[5]), float(m_matrix[13]),
		float(m_matrix[3]), float(m_matrix[7]), float(m_matrix[15])
	);
}

#endif

template<class T>
bool operator==(const TMatrix4<T> &lhs, const TMatrix4<T> &rhs)
{
	const T *lm = lhs.getMatrix();
	const T *rm = rhs.getMatrix();
	for (int32_t i = 0; i < 16; ++i)
	{
		if (lm[i] != rm[i])
			return false;
	}
	return true;
}

template<class T>
bool operator!=(const TMatrix4<T> &lhs, const TMatrix4<T> &rhs)
{
	return !(lhs == rhs);
}

template<class T>
TMatrix4<T> operator*(const TMatrix4<T> &lhs, const TMatrix4<T> &rhs)
{
	return TMatrix4<T>(lhs).combine(rhs);
}
