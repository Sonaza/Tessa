
template<class T>
Quaternion<T>::Quaternion()
	: x(0)
	, y(0)
	, z(0)
	, w(1)
{}

template<class T>
Quaternion<T>::Quaternion(T x, T y, T z, T w)
	: x(x)
	, y(y)
	, z(z)
	, w(w)
{}

template<class T>
inline bool Quaternion<T>::operator==(const Quaternion<T> &other) const
{
	return (x == other.x) &&
	       (y == other.y) &&
	       (z == other.z) &&
	       (w == other.w);
}

template<class T>
inline bool Quaternion<T>::operator!=(const Quaternion<T> &other) const
{
	return !(*this == other);
}

template<class T>
Quaternion<T>::Quaternion(const Quaternion<T> &other)
	: x(other.x)
	, y(other.y)
	, z(other.z)
	, w(other.w)
{
}

template<class T>
inline Quaternion<T> &Quaternion<T>::operator=(const Quaternion<T> &other)
{
	x = other.x;
	y = other.y;
	z = other.z;
	w = other.w;
	return *this;
}

template<class T>
inline Quaternion<T> Quaternion<T>::operator*(const Quaternion<T> &other) const
{
	Quaternion<T> q;
	q.w = (other.w * w) - (other.x * x) - (other.y * y) - (other.z * z);
	q.x = (other.w * x) + (other.x * w) + (other.y * z) - (other.z * y);
	q.y = (other.w * y) + (other.y * w) + (other.z * x) - (other.x * z);
	q.z = (other.w * z) + (other.z * w) + (other.x * y) - (other.y * x);
	return q;
}

template<class T>
inline Quaternion<T> &Quaternion<T>::operator*=(const Quaternion<T> &other)
{
	*this = (*this) * other;
	return *this;
}

template<class T>
inline Quaternion<T> Quaternion<T>::operator*(T scalar) const
{
	return Quaternion(x * scalar, y * scalar, z * scalar, w * scalar);
}

template<class T>
inline Quaternion<T> &Quaternion<T>::operator*=(T scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	w *= scalar;
	return *this;
}

template<class T>
inline Quaternion<T> Quaternion<T>::operator+(const Quaternion<T> &other) const
{
	return Quaternion(x + other.x, y + other.y, z + other.z, w + other.w);
}

template<class T>
inline void Quaternion<T>::getMatrix(TMatrix4<T> &m, const Vec3<T> &offset) const
{
	Quaternion<T> q(*this);
	q.normalize();

	m.m_matrix[0]  = 1 - 2 * q.y * q.y - 2 * q.z * q.z;
	m.m_matrix[1]  = 2 * q.x * q.y + 2 * q.z * q.w;
	m.m_matrix[2]  = 2 * q.x * q.z - 2 * q.y * q.w;
	m.m_matrix[3]  = 0;

	m.m_matrix[4]  = 2 * q.x * q.y - 2 * q.z * q.w;
	m.m_matrix[5]  = 1 - 2 * q.x * q.x - 2 * q.z * q.z;
	m.m_matrix[6]  = 2 * q.z * q.y + 2 * q.x * q.w;
	m.m_matrix[7]  = 0;

	m.m_matrix[8]  = 2 * q.x * q.z + 2 * q.y * q.w;
	m.m_matrix[9]  = 2 * q.z * q.y - 2 * q.x * q.w;
	m.m_matrix[10] = 1 - 2 * q.x * q.x - 2 * q.y * q.y;
	m.m_matrix[11] = 0;

	m.m_matrix[12] = offset.x;
	m.m_matrix[13] = offset.y;
	m.m_matrix[14] = offset.z;
	m.m_matrix[15] = 1;
}

template<class T>
inline void Quaternion<T>::getMatrixQuickly(TMatrix4<T> &m) const
{
	m.m_matrix[0]  = 1 - 2 * y * y - 2 * z * z;
	m.m_matrix[1]  = 2 * x * y + 2 * z * w;
	m.m_matrix[2]  = 2 * x * z - 2 * y * w;
	m.m_matrix[3]  = 0;

	m.m_matrix[4]  = 2 * x * y - 2 * z * w;
	m.m_matrix[5]  = 1 - 2 * x * x - 2 * z * z;
	m.m_matrix[6]  = 2 * z * y + 2 * x * w;
	m.m_matrix[7]  = 0;

	m.m_matrix[8]  = 2 * x * z + 2 * y * w;
	m.m_matrix[9]  = 2 * z * y - 2 * x * w;
	m.m_matrix[10] = 1 - 2 * x * x - 2 * y * y;
	m.m_matrix[11] = 0;

	m.m_matrix[12] = 0;
	m.m_matrix[13] = 0;
	m.m_matrix[14] = 0;
	m.m_matrix[15] = 1;
}

template<class T>
inline Quaternion<T> Quaternion<T>::getInverted() const
{
	return Quaternion(*this).invert();
}

template<class T>
inline Quaternion<T> &Quaternion<T>::invert()
{
	x = -x;
	y = -y;
	z = -z;
	return *this;
}

template<class T>
Quaternion<T> Quaternion<T>::makeFromEulerAngles(T x, T y, T z)
{
	double angle;

	angle = x * 0.5;
	const double sr = sin(angle);
	const double cr = cos(angle);

	angle = y * 0.5;
	const double sp = sin(angle);
	const double cp = cos(angle);

	angle = z * 0.5;
	const double sy = sin(angle);
	const double cy = cos(angle);

	const double cpcy = cp * cy;
	const double spcy = sp * cy;
	const double cpsy = cp * sy;
	const double spsy = sp * sy;

	Quaternion<T> q;
	q.x = (T)(sr * cpcy - cr * spsy);
	q.y = (T)(cr * spcy + sr * cpsy);
	q.z = (T)(cr * cpsy - sr * spcy);
	q.w = (T)(cr * cpcy + sr * spsy);
	return q.getNormalized();
}

template<class T>
Quaternion<T> Quaternion<T>::makeFromAngleAxis(T angle, const Vec3<T> &axis)
{
	const T halfAngle = angle * 0.5;
	const T halfAngleSin = sin(halfAngle);

	Quaternion<T> q;
	q.x = halfAngleSin * axis.x;
	q.y = halfAngleSin * axis.y;
	q.z = halfAngleSin * axis.z;
	q.w = cos(halfAngle);

	return q;
}

template<class T>
inline Quaternion<T> Quaternion<T>::makeFromEulerAngles(const Vec3<T> &angles)
{
	return makeFromEulerAngles(angles.x, angles.y, angles.z);
}

template<class T>
inline Vec3<T> Quaternion<T>::getRotated(const Vec3<T> &vector) const
{
	const Vec3<T> qvector(x, y, z);
	const Vec3<T> uv = qvector.cross(vector);
	const Vec3<T> uuv = qvector.cross(uv);
	return vector + ((uv * w) + uuv) * T(2);
}

template<class T>
inline T Quaternion<T>::dot(const Quaternion<T> &other) const
{
	return (x * other.x) + (y * other.y) + (z * other.z) + (w * other.w);
}

template<class T>
inline Quaternion<T> &Quaternion<T>::normalize()
{
	T slen = squareLength();
	TS_ASSERT(slen != T(0));
	if (slen != T(0))
	{
		*this *= T(1.0 / sqrt(slen));
	}
	return *this;
}

template<class T>
inline Quaternion<T> Quaternion<T>::getNormalized() const
{
	return Quaternion(*this).normalize();
}

template<class T>
inline Quaternion<T> &Quaternion<T>::normalizeWithZeroFailsafe(const Quaternion<T> &failsafe)
{
	T slen = squareLength();
	if (slen != 0)
	{
		*this = T(1 / sqrt(slen));
	}
	else
	{
		*this = failsafe;
	}
	return *this;
}

template<class T>
inline Quaternion<T> Quaternion<T>::getNormalizedWithZeroFailsafe(const Quaternion<T> &failsafe) const
{
	return Quaternion(*this).normalize(failsafe);
}

template<class T>
inline Quaternion<T> &Quaternion<T>::lerp(Quaternion<T> q1, Quaternion<T> q2, T time)
{
	*this = q1 * T(1.0 - time) + q2 * time;
	return *this;
}

template<class T>
inline Quaternion<T> &Quaternion<T>::slerp(Quaternion<T> q1, Quaternion<T> q2, T time, T threshold)
{
	T angle = q1.dot(q2);

	// make sure we use the short rotation
	if (angle < 0.0)
	{
		q1 *= -1.0;
		angle *= -1.0;
	}

	if (angle <= (1 - threshold))
	{
		// spherical interpolation
		const T theta = acos(angle);
		const T invsintheta = 1.0 / sin(theta);
		const T scale = sin(theta * (1.0 - time)) * invsintheta;
		const T invscale = sin(theta * time) * invsintheta;

		*this = (q1 * scale) + (q2 * invscale);
		return *this;
	}
	else
	{
		// linear interpolation
		return lerp(q1, q2, time);
	}
}

template<class T>
inline T Quaternion<T>::squareLength()
{
	return x * x + y * y + z * z + w * w;
}

template<class T>
inline T Quaternion<T>::length()
{
	return sqrt(x * x + y * y + z * z + w * w);
}

template<class T>
inline void Quaternion<T>::getAngleAxis(T &angle, Vec3<T> &axis) const
{
	const T scale = sqrt(x * x + y * y + z * z);

	if (scale == 0.0 || w > 1.0 || w < -1.0)
	{
		angle = 0.0;
		axis.x = 0.0;
		axis.y = 1.0;
		axis.z = 0.0;
	}
	else
	{
		const T invscale = 1.0 / scale;
		angle = 2.0 * acos(w);
		axis.x = x * invscale;
		axis.y = y * invscale;
		axis.z = z * invscale;
	}
}

template<class T>
inline Vec3<T> Quaternion<T>::getEulerAngles() const
{
	Vec3<T> euler;
	
	const double test = 2.0 * (y * w - x * z);

	if (floatEquals(test, 1.0))
	{
		// heading = rotation about z-axis
		euler.z = (T)(-2.0 * atan2(x, w));
		// bank = rotation about x-axis
		euler.x = 0.0;
		// attitude = rotation about y-axis
		euler.y = (T)(PI64 / 2.0);
	}
	else if (floatEquals(test, -1.0))
	{
		// heading = rotation about z-axis
		euler.z = (T)(2.0 * atan2(x, w));
		// bank = rotation about x-axis
		euler.x = 0.0;
		// attitude = rotation about y-axis
		euler.y = (T)(PI64 / -2.0);
	}
	else
	{
		const double sqx = x * x;
		const double sqy = y * y;
		const double sqz = z * z;
		const double sqw = w * w;

		// heading = rotation about z-axis
		euler.z = (T)atan2(2.0 * (x * y +z * w), (sqx - sqy - sqz + sqw));
		// bank = rotation about x-axis
		euler.x = (T)atan2(2.0 * (y * z +x * w), (-sqx - sqy + sqz + sqw));
		// attitude = rotation about y-axis
		euler.y = (T)asin(clamp(test, -1.0, 1.0));
	}

	return euler;
}

template<class T>
inline Quaternion<T> Quaternion<T>::makeFromRotation(const Vec3<T> &from, const Vec3<T> &to)
{
	const T d = from.dot(to);
	if (d >= 1.0) // If dot == 1, vectors are the same
	{
		return Quaternion<T>::identity;
	}
	else if (d <= -1.0) // exactly opposite
	{
		Vec3<T> axis(1.0, 0.0, 0.0);

		axis = axis.cross(from);
		if (axis.length() == 0.0)
		{
			axis = Vec3<T>(0.0, 1.0, 0.0);
			axis = axis.cross(from);
		}

		Quaternion<T> q;
		q.x = axis.x;
		q.y = axis.y;
		q.z = axis.z;
		q.w = 0.0;
		return q.getNormalized();
	}

	const T s = sqrt(T((1.0 + d) * 2.0));
	TS_ASSERT(s != 0.f);

	const Vec3<T> c = from.cross(to) * T(1.0 / s);

	Quaternion<T> q;
	q.x = c.x;
	q.y = c.y;
	q.z = c.z;
	q.w = s * 0.5f;
	return q.getNormalized();
}