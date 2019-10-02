
Quat::Quat()
	: x(0.f)
	, y(0.f)
	, z(0.f)
	, w(1.f)
{}

Quat::Quat(float x, float y, float z, float w)
	: x(x)
	, y(y)
	, z(z)
	, w(w)
{}

inline bool Quat::operator==(const Quat &other) const
{
	return (x == other.x) &&
	       (y == other.y) &&
	       (z == other.z) &&
	       (w == other.w);
}

inline bool Quat::operator!=(const Quat &other) const
{
	return !(*this == other);
}

Quat::Quat(const Quat &other)
	: x(other.x)
	, y(other.y)
	, z(other.z)
	, w(other.w)
{
}
	
inline Quat &Quat::operator=(const Quat &other)
{
	x = other.x;
	y = other.y;
	z = other.z;
	w = other.w;
	return *this;
}

inline Quat Quat::operator*(const Quat &other) const
{
	Quat q;
	q.w = (other.w * w) - (other.x * x) - (other.y * y) - (other.z * z);
	q.x = (other.w * x) + (other.x * w) + (other.y * z) - (other.z * y);
	q.y = (other.w * y) + (other.y * w) + (other.z * x) - (other.x * z);
	q.z = (other.w * z) + (other.z * w) + (other.x * y) - (other.y * x);
	return q;
}

inline Quat &Quat::operator*=(const Quat &other)
{
	*this = (*this) * other;
	return *this;
}

inline Quat Quat::operator*(float scalar) const
{
	return Quat(x * scalar, y * scalar, z * scalar, w * scalar);
}

inline Quat &Quat::operator*=(float scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	w *= scalar;
	return *this;
}

inline Quat Quat::operator+(const Quat &other) const
{
	return Quat(x + other.x, y + other.y, z + other.z, w + other.w);
}

inline void Quat::getMatrix(Mat4 &m, const VC3 &translation) const
{
	Quat q(*this);
	q.normalize();

	m.m_matrix[0]  = 1.f - 2.f * q.y * q.y - 2.f * q.z * q.z;
	m.m_matrix[1]  = 2.f * q.x * q.y + 2.f * q.z * q.w;
	m.m_matrix[2]  = 2.f * q.x * q.z - 2.f * q.y * q.w;
	m.m_matrix[3]  = 0.f;

	m.m_matrix[4]  = 2.f * q.x * q.y - 2.f * q.z * q.w;
	m.m_matrix[5]  = 1.f - 2.f * q.x * q.x - 2.f * q.z * q.z;
	m.m_matrix[6]  = 2.f * q.z * q.y + 2.f * q.x * q.w;
	m.m_matrix[7]  = 0.f;

	m.m_matrix[8]  = 2.f * q.x * q.z + 2.f * q.y * q.w;
	m.m_matrix[9]  = 2.f * q.z * q.y - 2.f * q.x * q.w;
	m.m_matrix[10] = 1.f - 2.f * q.x * q.x - 2.f * q.y * q.y;
	m.m_matrix[11] = 0.f;

	m.m_matrix[12] = translation.x;
	m.m_matrix[13] = translation.y;
	m.m_matrix[14] = translation.z;
	m.m_matrix[15] = 1.f;
}

//! Faster method to create a rotation matrix, you should normalize the Quat before!
inline void Quat::getMatrixFast(Mat4 &m) const
{
	m.m_matrix[0]  = 1.f - 2.f * y * y - 2.f * z * z;
	m.m_matrix[1]  = 2.f * x * y + 2.f * z * w;
	m.m_matrix[2]  = 2.f * x * z - 2.f * y * w;
	m.m_matrix[3]  = 0.f;

	m.m_matrix[4]  = 2.f * x * y - 2.f * z * w;
	m.m_matrix[5]  = 1.f - 2.f * x * x - 2.f * z * z;
	m.m_matrix[6]  = 2.f * z * y + 2.f * x * w;
	m.m_matrix[7]  = 0.f;

	m.m_matrix[8]  = 2.f * x * z + 2.f * y * w;
	m.m_matrix[9]  = 2.f * z * y - 2.f * x * w;
	m.m_matrix[10] = 1.f - 2.f * x * x - 2.f * y * y;
	m.m_matrix[11] = 0.f;

	m.m_matrix[12] = 0.f;
	m.m_matrix[13] = 0.f;
	m.m_matrix[14] = 0.f;
	m.m_matrix[15] = 1.f;
}


inline Quat Quat::getInverted() const
{
	return Quat(*this).invert();
}

inline Quat &Quat::invert()
{
	x = -x;
	y = -y;
	z = -z;
	return *this;
}

Quat Quat::makeFromEulerAngles(float x, float y, float z)
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

	Quat q;
	q.x = (float)(sr * cpcy - cr * spsy);
	q.y = (float)(cr * spcy + sr * cpsy);
	q.z = (float)(cr * cpsy - sr * spcy);
	q.w = (float)(cr * cpcy + sr * spsy);
	return q.getNormalized();
}

Quat Quat::makeFromAngleAxis(float angle, const VC3 &axis)
{
	const float halfAngle = angle * 0.5f;
	const float halfAngleSin = sin(halfAngle);

	Quat q;
	q.x = halfAngleSin * axis.x;
	q.y = halfAngleSin * axis.y;
	q.z = halfAngleSin * axis.z;
	q.w = cos(halfAngle);

	return q;
}

inline Quat Quat::makeFromEulerAngles(const VC3 &angles)
{
	return makeFromEulerAngles(angles.x, angles.y, angles.z);
}

inline VC3 Quat::getRotated(const VC3 &vector) const
{
	const VC3 qvec(x, y, z);
	VC3 uv = qvec.cross(vector) * (2.f * w);
	VC3 uuv = qvec.cross(uv) * 2.f;
	return vector + uv + uuv;
}

inline float Quat::dot(const Quat &other) const
{
	return (x * other.x) + (y * other.y) + (z * other.z) + (w * other.w);
}

inline Quat Quat::getNormalized() const
{
	return Quat(*this).normalize();
}

inline Quat &Quat::normalize()
{
	*this *= invSqrt(x * x + y * y + z * z + w * w);
	return *this;
}

inline Quat &Quat::lerp(Quat q1, Quat q2, float time)
{
	*this = q1 * (1.f - time) + (q2 * time);
	return *this;
}

inline Quat &Quat::slerp(Quat q1, Quat q2, float time, float threshold)
{
	float angle = q1.dot(q2);

	// make sure we use the short rotation
	if (angle < 0.f)
	{
		q1 *= -1.f;
		angle *= -1.f;
	}

	if (angle <= (1 - threshold))
	{
		// spherical interpolation
		const float theta = acos(angle);
		const float invsintheta = 1.f / sin(theta);
		const float scale = sin(theta * (1.f - time)) * invsintheta;
		const float invscale = sin(theta * time) * invsintheta;

		*this = (q1 * scale) + (q2 * invscale);
		return *this;
	}
	else
	{
		// linear interpolation
		return lerp(q1, q2, time);
	}
}

inline void Quat::getAngleAxis(float &angle, VC3 &axis) const
{
	const float scale = sqrt(x * x + y * y + z * z);

	if (scale == 0.f || w > 1.f || w < -1.f)
	{
		angle = 0.f;
		axis.x = 0.f;
		axis.y = 1.f;
		axis.z = 0.f;
	}
	else
	{
		const float invscale = 1.f / scale;
		angle = 2.f * acos(w);
		axis.x = x * invscale;
		axis.y = y * invscale;
		axis.z = z * invscale;
	}
}

inline VC3 Quat::getEulerAngles() const
{
	VC3 euler;
	
	const double test = 2.0 * (y * w - x * z);

	if (floatEquals(test, 1.0))
	{
		// heading = rotation about z-axis
		euler.z = (float)(-2.0 * atan2(x, w));
		// bank = rotation about x-axis
		euler.x = 0.f;
		// attitude = rotation about y-axis
		euler.y = (float)(PI64 / 2.0);
	}
	else if (floatEquals(test, -1.0))
	{
		// heading = rotation about z-axis
		euler.z = (float)(2.0 * atan2(x, w));
		// bank = rotation about x-axis
		euler.x = 0.f;
		// attitude = rotation about y-axis
		euler.y = (float)(PI64 / -2.0);
	}
	else
	{
		const double sqx = x * x;
		const double sqy = y * y;
		const double sqz = z * z;
		const double sqw = w * w;

		// heading = rotation about z-axis
		euler.z = (float) atan2(2.0 * (x * y +z * w), (sqx - sqy - sqz + sqw));
		// bank = rotation about x-axis
		euler.x = (float) atan2(2.0 * (y * z +x * w), (-sqx - sqy + sqz + sqw));
		// attitude = rotation about y-axis
		euler.y = (float) asin(clamp(test, -1.0, 1.0));
	}

	return euler;
}

inline Quat Quat::makeFromRotation(const VC3 &from, const VC3 &to)
{
	VC3 v0 = from;
	VC3 v1 = to;

	Quat q;

	const float d = v0.dot(v1);
	if (d >= 1.f) // If dot == 1, vectors are the same
	{
		// Returns an indentity quaternion
		return q;
	}
	else if (d <= -1.f) // exactly opposite
	{
		VC3 axis(1.f, 0.f, 0.f);

		axis = axis.cross(v0);
		if (axis.length() == 0.f)
		{
			axis = VC3(0.f, 1.f, 0.f);
			axis = axis.cross(v0);
		}

		q.x = axis.x;
		q.y = axis.y;
		q.z = axis.z;
		q.w = 0.f;
		return q.getNormalized();
	}

	const float s = sqrt((1.f + d) * 2.f);
	const VC3 c = v0.cross(v1) * (1.f / s);

	q.x = c.x;
	q.y = c.y;
	q.z = c.z;
	q.w = s * 0.5f;
	return q.getNormalized();
}
