
template <class T>
inline Vec3<T>::Vec3()
	: x(0), y(0), z(0)
{
}

template <class T>
inline Vec3<T>::Vec3(T v)
	: x(v), y(v), z(v)
{
}

template <class T>
inline Vec3<T>::Vec3(T x, T y, T z)
	: x(x), y(y), z(z)
{
}

template <class T>
template <class U>
inline Vec3<T>::Vec3(const Vec3<U> &vector)
	: x(static_cast<T>(vector.x))
	, y(static_cast<T>(vector.y))
	, z(static_cast<T>(vector.z))
{
}

// SFML conversions if using the library
#if defined(SFML_VERSION_MAJOR)

template <class T>
Vec3<T>::Vec3(const sf::Vector3<T> &vector)
	: x(vector.x)
	, y(vector.y)
	, z(vector.z)
{
}

template <class T>
inline Vec3<T>::operator sf::Vector3<T>()
{
	return sf::Vector3<T>(x, y, z);
}

#endif

template <class T>
inline float Vec3<T>::length() const
{
	return std::sqrt(squarelength());
}

template <class T>
inline float Vec3<T>::squarelength() const
{
	return x * x + y * y + z * z;
}

template <class T>
inline Vec3<T> &Vec3<T>::normalize()
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
inline Vec3<T> &Vec3<T>::getNormalized()
{
	return Vec3<T>(*this).normalize();
}

template <class T>
inline float Vec3<T>::dot(const Vec3<T> &b)
{
	return x * b.x + y * b.y + z * b.z;
}

template <class T>
inline Vec3<T> &Vec3<T>::cross(const Vec3<T> &b)
{
	*this = Vec3<T>(
		y * b.z - z * b.y,
		z * b.x - x * b.z,
		x * b.y - y * b.x
	);

	return *this;
}

template <class T>
inline Vec3<T> operator*(const Vec3<T> &lhs, const Vec3<T> &rhs)
{
	return Vec3<T>(
		lhs.x * rhs.x,
		lhs.y * rhs.y,
		lhs.z * rhs.z
	);
}

template <class T>
inline Vec3<T> operator*(const Vec3<T> &lhs, T v)
{
	return Vec3<T>(
		lhs.x * v,
		lhs.y * v,
		lhs.z * v
	);
}

template <class T>
inline Vec3<T> operator*(float v, const Vec3<T> &rhs)
{
	return rhs * v;
}

template <class T>
inline Vec3<T> &operator*=(Vec3<T> &lhs, const Vec3<T> &rhs)
{
	lhs = lhs * rhs;
	return lhs;
}

template <class T>
inline Vec3<T> &operator*=(Vec3<T> &lhs, T v)
{
	lhs = lhs * v;
	return lhs;
}

template <class T>
inline Vec3<T> operator/(const Vec3<T> &lhs, const Vec3<T> &rhs)
{
	return Vec3<T>(
		lhs.x / rhs.x,
		lhs.y / rhs.y,
		lhs.z / rhs.z
	);
}

template <class T>
inline Vec3<T> operator/(const Vec3<T> &lhs, T v)
{
	return Vec3<T>(
		lhs.x / v,
		lhs.y / v,
		lhs.z / v
	);
}

template <class T>
inline Vec3<T> &operator/=(Vec3<T> &lhs, const Vec3<T> &rhs)
{
	lhs = lhs / rhs;
	return lhs;
}

template <class T>
inline Vec3<T> &operator/=(Vec3<T> &lhs, T v)
{
	lhs = lhs / v;
	return lhs;
}

template <class T>
inline Vec3<T> operator+(const Vec3<T> &lhs, const Vec3<T> &rhs)
{
	return Vec3<T>(
		lhs.x + rhs.x,
		lhs.y + rhs.y,
		lhs.z + rhs.z
	);
}

template <class T>
inline Vec3<T> &operator+=(Vec3<T> &lhs, const Vec3<T> &rhs)
{
	lhs = lhs + rhs;
	return lhs;
}

template <class T>
inline Vec3<T> operator-(const Vec3<T> &lhs, const Vec3<T> &rhs)
{
	return Vec3<T>(
		lhs.x - rhs.x,
		lhs.y - rhs.y,
		lhs.z - rhs.z
	);
}

template <class T>
inline Vec3<T> &operator-=(Vec3<T> &lhs, const Vec3<T> &rhs)
{
	lhs = lhs - rhs;
	return lhs;
}

template <class T>
inline Vec3<T> operator-(const Vec3<T> &v)
{
	return v * -1.f;
}

template <class T>
inline bool operator==(const Vec3<T> &lhs, const Vec3<T> &rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

template <class T>
inline bool operator!=(const Vec3<T> &lhs, const Vec3<T> &rhs)
{
	return (lhs.x != rhs.x) || (lhs.y != rhs.y) || (lhs.z != rhs.z);
}

template <class T>
inline Vec3<T> normalize(const Vec3<T> &v)
{
	return Vec3<T>(v).normalize();
}

template <class T>
inline float length(const Vec3<T> &v)
{
	return v.length();
}

template <class T>
inline float dot(const Vec3<T> &a, const Vec3<T> &b)
{
	return Vec3<T>(a).dot(b);
}

template <class T>
inline Vec3<T> cross(const Vec3<T> &a, const Vec3<T> &b)
{
	return Vec3<T>(a).cross(b);
}
