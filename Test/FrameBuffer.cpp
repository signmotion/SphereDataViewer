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
			const float dr = avgD / halfWidth;
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


void CFrameBuffer::RenderSphere2(const FrameRenderElement& fre)
{
	const float halfWidth = m_iWidth / 2;
	const float centerX = fre.screenX * halfWidth + halfWidth;
	const float centerY = fre.screenY * halfWidth + halfWidth;

	const float radius = fre.screenRadius * halfWidth;
	const float radius2 = radius * radius;

	//const DirectShading shading{ fre };
	const PhongShading shading{ fre, radius };


	// take from pool or create a new template circle
	const int r2 = radius * 2;
	const int sizeSide = (r2 + 1) * 2;
	const int sizeBox = sizeSide * sizeSide;
	auto ft = poolCircles.find(sizeBox);
	static constexpr int UNDEFINED_X = std::numeric_limits< int >::min();
	static constexpr coord_t UNDEFINED_COORD = { UNDEFINED_X, UNDEFINED_X };
	if (ft == poolCircles.cend()) {
		circle_t circle;
		circle.resize(sizeBox, UNDEFINED_COORD);
		for (int x = 0; x <= r2 * 2; ++x)
		{
			const int dx = x - r2;
			const int dx2 = dx * dx;
			for (int y = 0; y <= r2 * 2; ++y)
			{
				const int dy = y - r2;
				const int dy2 = dy * dy;
				if (dx2 + dy2 <= radius2)
				{
					const int pi = x + y * sizeSide;
					circle[pi] = { dx, dy };
				}
			}
		}
		ft = poolCircles.emplace(sizeBox, circle).first;
	}
	const circle_t& circle = ft->second;


	std::for_each(
		std::execution::par,
		std::begin(circle),
		std::end(circle),
		[this, &fre, centerX, centerY, radius, radius2, r2,
		halfWidth, sizeSide, &shading](const coord_t& coord)
		{
			if (coord.first == UNDEFINED_COORD.first)
			{
				return;
			}

			const int dx = coord.first;
			const int x = centerX + dx;
			if (x < 0 || x >= m_iWidth)
				return;

			const int dy = coord.second;
			const int y = centerY + dy;
			if (y < 0 || y >= m_iHeight)
				return;

			const int dx2 = dx * dx;
			const int dy2 = dy * dy;

			// smooth a 2D circle to 3D
			const float avgD = sqrtf(dx2 + dy2);
			// faster but more dirt
			//const float avgD = (std::abs(dx) + std::abs(dy)) / 2;
			const float dr = avgD / halfWidth;
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
		});
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
