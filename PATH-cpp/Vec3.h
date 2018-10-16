#pragma once
#include "math/Vec2.h"

namespace math
{
	class Vec3
	{
	private:
		float elements[3];

	public:
		Vec3(float x = 0, float y = 0, float z = 0);
		Vec3(const Vec2& v, float z = 0);
		
		friend Vec3 operator-(const Vec3& v1, const Vec3& v2);
		friend Vec3 operator+(const Vec3& v1, const Vec3& v2);
		friend Vec3 operator*(const Vec3& v, float f);
		friend Vec3 operator*(float f, const Vec3& v);
		friend Vec3 operator/(const Vec3& v, float f);

		Vec3& operator-=(const Vec3& rhs);
		Vec3& operator+=(const Vec3& rhs);
		Vec3& operator*=(float f);
		Vec3  operator- ();
		Vec3& operator= (const Vec3& rhs);

		// dot product
		friend float operator*(const Vec3& v1, const Vec3& v2);

		friend bool operator==(const Vec3& v1, const Vec3& v2);

		// comparing magnitudes
		friend bool operator< (const Vec3&, const Vec3&);
		friend bool operator> (const Vec3&, const Vec3&);
		friend bool operator<=(const Vec3&, const Vec3&);
		friend bool operator>=(const Vec3&, const Vec3&);

		float length() const;
		bool isZero() const;
		Vec3& scaleTo(float targetLength);

		float& operator[](const int& i) { return elements[i]; }
		float  operator[](const int& i) const { return elements[i]; }
		float& x() { return elements[0]; }
		float& y() { return elements[1]; }
		float& z() { return elements[2]; }
		const float& x() const { return elements[0]; }
		const float& y() const { return elements[1]; }
		const float& z() const { return elements[2]; }
	};
}