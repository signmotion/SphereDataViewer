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
CFrameBuffer::CFrameBuffer(const int iWidth, const int iHeight)
	:m_iWidth(iWidth), m_iHeight(iHeight)
{
	m_FramebufferArray.resize(iWidth * iHeight, 0);

	Light.normalize();
}


CFrameBuffer::~CFrameBuffer()
{
}


void CFrameBuffer::Clear()
{
	memset(&m_FramebufferArray[0], 0, m_iWidth * m_iHeight * 4);

	//Light.x -= 0.02f; // some light animation
	//Light = Light.normalize();
}


const unsigned int* CFrameBuffer::GetFrameBuffer() const
{
	return static_cast<const unsigned int*>(&m_FramebufferArray[0]);
};


void CFrameBuffer::RenderSphere(
	float fScreenX,
	float fScreenY,
	float fScreenZ,
	float fScreenRadius,
	unsigned int ARGB)
{
	const float centerX = fScreenX * m_iWidth / 2 + m_iWidth / 2;
	const float centerY = fScreenY * m_iWidth / 2 + m_iWidth / 2;

	const float radius = fScreenRadius * m_iWidth / 2;
	const float radius2 = radius * radius;

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

			// Phong shading
			{
				Vec3 vec_normal = {
					(float)dx,
					(float)dy,
					sqrtf(radius2 - (dx2 + dy2)) };
				vec_normal.normalize();

				const float NdotL = Light.dot(vec_normal);
				if (NdotL > 0)
				{
					const Vec3 vec_eye = {
						Light.x() + fScreenX,
						Light.y() + fScreenY,
						Light.z() + 1.f };
					const Vec3 vec_half = vec_eye.normalizeCopy();

					const float NdotHV = vec_half.dot(vec_normal);
					static constexpr float shininess = 12;
					const float specular = pow(NdotHV, shininess);
					float alpha = (NdotL + specular);
					if (alpha > 1.0f)
						alpha = 1.0f;

					float r = ((ARGB & 0xFF0000) >> 16) * alpha;
					float g = ((ARGB & 0x00FF00) >> 8) * alpha;
					float b = ((ARGB & 0x0000FF >> 0)) * alpha;
					r = std::min(r, 255.f);
					g = std::min(g, 255.f);
					b = std::min(b, 255.f);

					const int i = x + y * m_iWidth;
					const int color = (int(r) << 16) | (int(g) << 8) | (int(b) << 0);
					m_FramebufferArray[i] = color;
				}
			}
		}
	}
}
