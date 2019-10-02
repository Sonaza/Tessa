
template <class T>
TS_FORCEINLINE Vec4<T>::Vec4()
	: x(0), y(0), z(0), w(1)
{
}

template <class T>
TS_FORCEINLINE Vec4<T>::Vec4(T x, T y, T z, T w)
	: x(x), y(y), z(z), w(w)
{
}

template <class T>
Vec4<T>::Vec4(Vec3<T> v, T w)
	: x(v.x), y(v.y), z(v.z), w(w)
{
}

template <class T>
Vec4<T>::Vec4(const T v[4])
	: x(v[0]), y(v[1]), z(v[2]), w(v[3])
{
}

template <class T>
template <class U>
Vec4<T>::Vec4(const Vec4<U> &vector)
	: x(static_cast<T>(vector.x))
	, y(static_cast<T>(vector.y))
	, z(static_cast<T>(vector.z))
	, w(static_cast<T>(vector.w))
{
}

template <class T>
T &Vec4<T>::operator[](SizeType index)
{
	TS_ASSERTF(index < 4, "Index out of range");
	return v[index];
}

template <class T>
const T &Vec4<T>::operator[](SizeType index) const
{
	TS_ASSERTF(index < 4, "Index out of range");
	return v[index];
}

template <class T>
inline float Vec4<T>::length() const
{
	return std::sqrt(x * x + y * y + z * z);
}

template <class T>
inline float Vec4<T>::squarelength() const
{
	return x * x + y * y + z * z;
}

template <class T>
inline Vec4<T> &Vec4<T>::normalize()
{
	float len = length();
	if (len != 0.f)
	{
		x /= len;
		y /= len;
		z /= len;
	}
	return *this;
}

template <class T>
inline Vec4<T> &Vec4<T>::getNormalized()
{
	return Vec4<T>(*this).normalize();
}

template <class T>
inline float Vec4<T>::dot(const Vec4<T> &other) const
{
	return x * other.x + y * other.y + z * other.z;
}

template <class T>
inline Vec4<T> Vec4<T>::cross(const Vec4<T> &other) const
{
	return Vec4<T>(
		y * other.z - z * other.y,
		z * other.x - x * other.z,
		x * other.y - y * other.x
	);
}

template <class T>
inline Vec4<T> operator*(const Vec4<T> &lhs, const Vec4<T> &rhs)
{
	return Vec4<T>(
		lhs.x * rhs.x,
		lhs.y * rhs.y,
		lhs.z * rhs.z
	);
}

template <class T>
inline Vec4<T> operator*(const Vec4<T> &lhs, T v)
{
	return Vec4<T>(
		lhs.x * v,
		lhs.y * v,
		lhs.z * v
	);
}

template <class T>
inline Vec4<T> operator*(float v, const Vec4<T> &rhs)
{
	return rhs * v;
}

template <class T>
inline Vec4<T> &operator*=(Vec4<T> &lhs, const Vec4<T> &rhs)
{
	lhs = lhs * rhs;
	return lhs;
}

template <class T>
inline Vec4<T> &operator*=(Vec4<T> &lhs, T v)
{
	lhs = lhs * v;
	return lhs;
}

template <class T>
inline Vec4<T> operator/(const Vec4<T> &lhs, const Vec4<T> &rhs)
{
	return Vec4<T>(
		lhs.x / rhs.x,
		lhs.y / rhs.y,
		lhs.z / rhs.z
	);
}

template <class T>
inline Vec4<T> operator/(const Vec4<T> &lhs, T v)
{
	return Vec4<T>(
		lhs.x / v,
		lhs.y / v,
		lhs.z / v
	);
}

template <class T>
inline Vec4<T> &operator/=(Vec4<T> &lhs, const Vec4<T> &rhs)
{
	lhs = lhs / rhs;
	return lhs;
}

template <class T>
inline Vec4<T> &operator/=(Vec4<T> &lhs, T v)
{
	lhs = lhs / v;
	return lhs;
}

template <class T>
inline Vec4<T> operator+(const Vec4<T> &lhs, const Vec4<T> &rhs)
{
	return Vec4<T>(
		lhs.x + rhs.x,
		lhs.y + rhs.y,
		lhs.z + rhs.z
	);
}

template <class T>
inline Vec4<T> &operator+=(Vec4<T> &lhs, const Vec4<T> &rhs)
{
	lhs = lhs + rhs;
	return lhs;
}

template <class T>
inline Vec4<T> operator-(const Vec4<T> &lhs, const Vec4<T> &rhs)
{
	return Vec4<T>(
		lhs.x - rhs.x,
		lhs.y - rhs.y,
		lhs.z - rhs.z
	);
}

template <class T>
inline Vec4<T> &operator-=(Vec4<T> &lhs, const Vec4<T> &rhs)
{
	lhs = lhs - rhs;
	return lhs;
}

template <class T>
inline Vec4<T> operator-(const Vec4<T> &v)
{
	return v * -1.f;
}

template <class T>
inline bool operator==(const Vec4<T> &lhs, const Vec4<T> &rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w;
}

template <class T>
inline bool operator!=(const Vec4<T> &lhs, const Vec4<T> &rhs)
{
	return (lhs.x != rhs.x) || (lhs.y != rhs.y) || (lhs.z != rhs.z) || (lhs.w != rhs.w);
}
