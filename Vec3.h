#pragma once

#include <algorithm>


//! \brief Standard 3D-vector.
class Vec3
{
public:
	Vec3() :
		Vec3(0.f, 0.f, 0.f)
	{}

	Vec3(const Vec3& b) :
		Vec3(b.x(), b.y(), b.z())
	{}

	Vec3(float x, float y, float z) :
		m_x(x), m_y(y), m_z(z)
	{}

	Vec3& operator=(const Vec3& b) {
		m_x = b.m_x;
		m_y = b.m_y;
		m_z = b.m_z;
		return *this;
	}

	float x() const {
		return m_x;
	}

	float y() const {
		return m_y;
	}

	float z() const {
		return m_z;
	}

	Vec3 normalizeCopy() const
	{
		const float d = 1.f / length();
		return Vec3{ m_x * d, m_y * d, m_z * d };
	}

	void normalize()
	{
		const float d = 1.f / length();
		m_x *= d;
		m_y *= d;
		m_z *= d;
	}

	float length() const
	{
		return sqrtf(m_x * m_x + m_y * m_y + m_z * m_z);
	}

	float dot(const Vec3& v2) const
	{
		return m_x * v2.m_x + m_y * v2.m_y + m_z * v2.m_z;
	}

private:
	float m_x;
	float m_y;
	float m_z;
};
