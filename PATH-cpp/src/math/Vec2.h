#pragma once

namespace math {

	class Vec2
	{
	private:
		float elements[2];

	public:
		Vec2(float x = 0, float y = 0);
		// copy constructor is default

		friend Vec2 operator+(const Vec2& v1, const Vec2& v2);
		friend Vec2 operator-(const Vec2& v1, const Vec2& v2);
		friend Vec2 operator*(const Vec2& v, float f);
		friend Vec2 operator*(float f, const Vec2& v);
		friend Vec2 operator/(const Vec2& v, float f);

		Vec2& operator+=(const Vec2& rhs);
		Vec2& operator-=(const Vec2& rhs);
		Vec2& operator*=(float factor);
		Vec2  operator-();
		Vec2& operator=(const Vec2& rhs);

		// dot product
		friend float operator*(const Vec2& v1, const Vec2& v2);

		//friend std::ostream& operator<<(std::ostream& out, const Vec2& v);

		friend bool operator==(const Vec2&, const Vec2&);

		// comparing magnitudes
		friend bool operator<(const Vec2&, const Vec2&);
		friend bool operator>(const Vec2&, const Vec2&);
		friend bool operator<=(const Vec2&, const Vec2&);
		friend bool operator>=(const Vec2&, const Vec2&);

		// polar mutators
		Vec2& setPolar(float length, float angle);
		Vec2& rotateBy(float angle);
		Vec2& rotateTo(float targetAngle);
		Vec2& scaleTo(float targetLength);

		// rectangular accessors
		float& operator[](const int& i) { return elements[i]; }
		float  operator[](const int& i) const { return elements[i]; }

		float& x() { return elements[0]; }
		float& y() { return elements[1]; }
		const float& x() const { return elements[0]; }
		const float& y() const { return elements[1]; }

		// polar accessors
		float length() const;
		float angle() const; // returns 0 if isZero

		bool isZero() const;
	};

} /* namespace math */