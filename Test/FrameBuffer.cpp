#include "FrameBuffer.h"

#include <math.h>
#include <algorithm>


struct Vec3
{
	float x, y, z;

	Vec3 normalize() const
	{
		const float d = 1.f / sqrt(x * x + y * y + z * z);
		Vec3 vec;
		vec.x = x * d;
		vec.y = y * d;
		vec.z = z * d;
		return vec;
	}

	float dot(const Vec3& v2) const
	{
		return x * v2.x + y * v2.y + z * v2.z;
	}
};


// Global light
Vec3 Light;


//////////////////////////////////////////////////////////////////////////
CFrameBuffer::CFrameBuffer(const int iWidth, const int iHeight)
	:m_iWidth(iWidth), m_iHeight(iHeight)
{
	m_FramebufferArray.resize(iWidth * iHeight, 0);

	Light.x = 1.f;
	Light.y = -0.5f;
	Light.z = 0.7f;
	Light = Light.normalize();
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
				Vec3 vec_normal;
				vec_normal.x = (float)dx;
				vec_normal.y = (float)dy;
				vec_normal.z = sqrt(radius2 - (dx2 + dy2));
				vec_normal = vec_normal.normalize();

				const float NdotL = Light.dot(vec_normal);
				if (NdotL > 0)
				{
					Vec3 vec_eye;
					vec_eye.x = Light.x + fScreenX;
					vec_eye.y = Light.y + fScreenY;
					vec_eye.z = Light.z + 1.f;
					Vec3 vec_half = vec_eye.normalize();

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
