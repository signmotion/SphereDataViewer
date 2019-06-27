#pragma once

#include <algorithm>


//! \brief Standard 3D-vector.
class Vec3
{
public:
	float x;
	float y;
	float z;


public:
	Vec3() :
		Vec3(0.f, 0.f, 0.f)
	{}

	Vec3(const Vec3& b) :
		Vec3(b.x, b.y, b.z)
	{}

	Vec3(float x, float y, float z) :
		x(x), y(y), z(z)
	{}

	Vec3& operator=(const Vec3& b) {
		x = b.x;
		y = b.y;
		z = b.z;
		return *this;
	}

	Vec3 normalizeCopy() const
	{
		const float d = 1.f / length();
		return Vec3{ x * d, y * d, z * d };
	}

	void normalize()
	{
		const float d = 1.f / length();
		x *= d;
		y *= d;
		z *= d;
	}

	float length() const
	{
		return sqrtf(x * x + y * y + z * z);
	}

	float dot(const Vec3& v2) const
	{
		return x * v2.x + y * v2.y + z * v2.z;
	}
};
