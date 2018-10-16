#include "Vec3.h"
#include <cmath>

namespace math
{

	Vec3::Vec3(float x, float y, float z) // parameters default to 0
	{
		elements[0] = x;
		elements[1] = y;
		elements[2] = z;
	}

	Vec3::Vec3(const Vec2& v, float z)
	: Vec3(v.x(), v.y(), z)
	{
	}

	Vec3 operator+(const Vec3& v1, const Vec3& v2)
	{
		return Vec3(v1.x() + v2.x(), v1.y() + v2.y());
	}

	Vec3 operator-(const Vec3& v1, const Vec3& v2)
	{
		return Vec3(v1.x() - v2.x(), v1.y() - v2.y());
	}

	Vec3 operator*(const Vec3& v, float f)
	{
		return Vec3(v.x() * f, v.y() * f, v.z() * f);
	}
	Vec3 operator*(float f, const Vec3& v)
	{
		return v * f;
	}

	Vec3 operator/(const Vec3& v, float f)
	{
		return 1.0f / f * v;
	}

	Vec3& Vec3::operator+=(const Vec3& rhs)
	{
		x() += rhs.x();
		y() += rhs.y();
		return *this;
	}

	Vec3& Vec3::operator-=(const Vec3& rhs)
	{
		x() -= rhs.x();
		y() -= rhs.y();
		z() -= rhs.z();
		return *this;
	}

	Vec3& Vec3::operator*=(float factor)
	{
		x() *= factor;
		y() *= factor;
		z() *= factor;
		return *this;
	}

	Vec3 Vec3::operator-()
	{
		return -1.0 * (*this);
	}

	Vec3& Vec3::operator=(const Vec3 & rhs)
	{
		x() = rhs.x();
		y() = rhs.y();
		z() = rhs.z();
		return *this;
	}

	// Dot product
	float operator*(const Vec3& v1, const Vec3& v2)
	{
		return v1.x() * v2.x() + v1.y() * v2.y() + v1.z() * v2.z();
	}

	//std::ostream& operator<<(std::ostream& out, const Vec3& v)
	//{
	//	out << "Vec3(" << v.x << ", " << v.y << ")";
	//	return out;
	//}

	bool operator==(const Vec3& lhs, const Vec3& rhs)
	{
		return (lhs.x() == rhs.x()) && (lhs.y() == rhs.y() && (lhs.z() == rhs.z()));
	}

	// length comparison
	bool operator<(const Vec3& lhs, const Vec3& rhs)
	{
		return lhs.length() < rhs.length();
	}
	bool operator>(const Vec3& lhs, const Vec3& rhs)
	{
		return rhs < lhs;
	}
	bool operator<=(const Vec3& lhs, const Vec3& rhs)
	{
		return !(lhs > rhs);
	}
	bool operator>=(const Vec3& lhs, const Vec3& rhs)
	{
		return !(lhs < rhs);
	}

	Vec3& Vec3::scaleTo(float targetLength)
	{
		// special case
		if (isZero())
			x() = targetLength;
		else
			*this *= (targetLength / length());

		return *this;
	}

	float Vec3::length() const
	{
		return std::sqrt((*this) * (*this));
	}

	bool Vec3::isZero() const
	{
		return (x() == 0) && (y() == 0) && (z() == 0);
	}

} /* namespace math */