#include "FrameBuffer.h"

#include <math.h>
#include <algorithm>


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


// Global light
Vec3 Light = { 1.f, -0.5f, 0.7f };


//////////////////////////////////////////////////////////////////////////
CFrameBuffer::CFrameBuffer(const int iWidth, const int iHeight) :
	m_iWidth(iWidth),
	m_iHeight(iHeight)
{
	const int size = iWidth * iHeight;
	m_FramebufferArray.resize(size, 0);
	m_ZBuffer.resize(size, 0);

	Light.normalize();
}


CFrameBuffer::~CFrameBuffer()
{
}


void CFrameBuffer::Clear()
{
	const int size = m_iWidth * m_iHeight;
	memset(std::data(m_FramebufferArray), 0,
		size * sizeof(frameBuffer_t::value_type));
	std::fill(std::begin(m_ZBuffer), std::end(m_ZBuffer),
		std::numeric_limits< zBuffer_t::value_type >::max());
}


const unsigned int* CFrameBuffer::GetFrameBuffer() const
{
	return static_cast<const unsigned int*>(std::data(m_FramebufferArray));
};


void CFrameBuffer::RenderSphere(
	float fScreenX,
	float fScreenY,
	float fScreenZ,
	float fScreenRadius,
	color_t ARGB)
{
	const float halfWidth = m_iWidth / 2;
	const float centerX = fScreenX * halfWidth + halfWidth;
	const float centerY = fScreenY * halfWidth + halfWidth;

	const float radius = fScreenRadius * halfWidth;
	const float radius2 = radius * radius;

	//const DirectShading shading{ ARGB };
	const PhongShading shading{
		fScreenX, fScreenY, fScreenZ, fScreenRadius, radius, ARGB };

	for (int x = centerX - radius * 2; x <= centerX + radius * 2; ++x)
	{
		if (x < 0 || x >= m_iWidth)
			continue;

		const int dx = x - centerX;
		const int dx2 = dx * dx;

		for (int y = centerY - radius * 2; y <= centerY + radius * 2; ++y)
		{
			if (y < 0 || y >= m_iHeight)
				continue;

			const int dy = y - centerY;
			const int dy2 = dy * dy;
			if (dx2 + dy2 > radius2)
				continue;

			// smooth a 2D circle to 3D
			const float avgD = sqrtf(dx2 + dy2);
			// faster but more dirt
			//const float avgD = (std::abs(dx) + std::abs(dy)) / 2;
			const float dr = (avgD / radius) * radius / halfWidth;
			const float fScreenZ3D = fScreenZ + dr;

			const int i = x + y * m_iWidth;
			if (m_ZBuffer[i] > fScreenZ3D)
			{
				const Shading::color_t color = shading(dx, dy);
				if (Shading::IsDefinedColor(color))
				{
					std::lock_guard guard(mutex);
					m_FramebufferArray[i] = color;
					m_ZBuffer[i] = fScreenZ3D;
				}
			} // if fScreenZ

		} // for y
	} // for x
}




Shading::color_t DirectShading::operator()(int, int) const
{
	return GetBaseColor();
}




Shading::color_t PhongShading::operator()(int x, int y) const
{
	Vec3 vec_normal = {
		(float)x,
		(float)y,
		sqrtf(m_frameRadius * m_frameRadius - (x * x + y * y)) };
	vec_normal.normalize();

	const float NdotL = Light.dot(vec_normal);
	color_t color = GetUndefinedColor();
	if (NdotL > 0)
	{
		const Vec3 vec_eye = {
			Light.x() + m_screenX,
			Light.y() + m_screenY,
			Light.z() + 1.f };
		const Vec3 vec_half = vec_eye.normalizeCopy();

		const float NdotHV = vec_half.dot(vec_normal);
		static constexpr float shininess = 12;
		const float specular = pow(NdotHV, shininess);
		float alpha = (NdotL + specular);
		if (alpha > 1.0f)
			alpha = 1.0f;

		float r = ((GetBaseColor() & 0xFF0000) >> 16) * alpha;
		float g = ((GetBaseColor() & 0x00FF00) >> 8) * alpha;
		float b = ((GetBaseColor() & 0x0000FF >> 0)) * alpha;
		r = std::min(r, 255.f);
		g = std::min(g, 255.f);
		b = std::min(b, 255.f);

		color = (int(r) << 16) | (int(g) << 8) | (int(b) << 0);
	} // if NdotL > 0

	return color;
}
