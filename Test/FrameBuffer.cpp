#include "FrameBuffer.h"
#include "../Vec3.h"
#include "../Vec3SIMD.h"

#include <math.h>
#include <algorithm>
#include <execution>

// change a vector
typedef Vec3SIMD vec_t;
//typedef Vec3 vec_t;

// Global light
vec_t Light = { 1.f, -0.5f, 0.7f };


//////////////////////////////////////////////////////////////////////////
CFrameBuffer::CFrameBuffer(int iWidth, int iHeight) :
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
	std::fill(
		std::execution::par,
		std::begin(m_ZBuffer),
		std::end(m_ZBuffer),
		std::numeric_limits< zBuffer_t::value_type >::max());
}


const CFrameBuffer::color_t* CFrameBuffer::GetFrameBuffer() const
{
	return static_cast<const color_t*>(std::data(m_FramebufferArray));
};


void CFrameBuffer::RenderSphere(const FrameRenderElement& fre)
{
	const float halfWidth = m_iWidth / 2;
	const float centerX = fre.screenX * halfWidth + halfWidth;
	const float centerY = fre.screenY * halfWidth + halfWidth;

	const float radius = fre.screenRadius * halfWidth;
	const float radius2 = radius * radius;

	//const DirectShading shading{ fre };
	const PhongShading shading{ fre, radius };

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
			const float fScreenZ3D = fre.screenZ + dr;

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
			} // if fScreenZ3D

		} // for y
	} // for x
}




Shading::color_t DirectShading::operator()(int, int) const
{
	return GetBaseColor();
}




Shading::color_t PhongShading::operator()(int x, int y) const
{
	vec_t vec_normal = {
		(float)x,
		(float)y,
		sqrtf(m_frameRadius * m_frameRadius - (x * x + y * y)) };
	vec_normal.normalize();

	const float NdotL = Light.dot(vec_normal);
	color_t color = GetUndefinedColor();
	if (NdotL > 0)
	{
		const FrameRenderElement& fre = GetFrameRenderElement();
		const vec_t vec_eye = {
			Light.x + fre.screenX,
			Light.y + fre.screenY,
			Light.z + 1.f };
		const vec_t vec_half = vec_eye.normalizeCopy();

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
