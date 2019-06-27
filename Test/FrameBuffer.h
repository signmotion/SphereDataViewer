#pragma once

#include <vector>
#include <mutex>


class Shading;


class CFrameBuffer
{
public:
	typedef unsigned int color_t;
	typedef std::vector< color_t > frameBuffer_t;
	typedef std::vector< float > zBuffer_t;

	static constexpr color_t UNDEFINED_COLOR = 0x00000000;


public:
	CFrameBuffer(int iWidth, int iHeight);

	~CFrameBuffer();

	void Clear();

	//! \param fScreenX [-1..1]
	//! \param fScreenY [-1..1]
	//! \param fScreenZ ]0..1[
	//! \param fScreenRadius >0 (-1..1 = 2 means full screen)
	void RenderSphere(
		float fScreenX,
		float fScreenY,
		float fScreenZ,
		float fScreenRadius,
		color_t ARGB);

	const color_t* GetFrameBuffer() const;
	int GetWidth() const { return m_iWidth; }
	int GetHeight() const { return m_iHeight; }


private:
	//! \return Color for coords.
	color_t color(Shading*);


private:
	frameBuffer_t m_FramebufferArray;
	zBuffer_t m_ZBuffer;
	int m_iWidth;
	int m_iHeight;

	std::mutex mutex;
};




//! \brief Base class for shading.
class Shading {
public:
	typedef CFrameBuffer::color_t color_t;


public:
	explicit Shading(color_t baseColor) :
		m_baseColor(baseColor)
	{}

	virtual ~Shading()
	{}

	color_t GetBaseColor() const
	{
		return m_baseColor;
	}

	static color_t GetUndefinedColor()
	{
		return CFrameBuffer::UNDEFINED_COLOR;
	}

	//! \brief Detect when a color is undefined.
	//! \see IsDefinedColor()
	static bool IsUndefinedColor(color_t color)
	{
		return color == GetUndefinedColor();
	}

	//! \brief Detect when a color is defined.
	//! \see IsUndefinedColor()
	static bool IsDefinedColor(color_t color)
	{
		return color != GetUndefinedColor();
	}

	//! \param x Coord in a frame buffer.
	//! \param y Coord in a frame buffer.
	virtual color_t operator()(int x, int y) const = 0;


private:
	color_t m_baseColor;
};




class DirectShading : public Shading {
public:
	explicit DirectShading(color_t baseColor) :
		Shading(baseColor)
	{}

	virtual color_t operator()(int x, int y) const override;
};




class PhongShading : public Shading {
public:
	PhongShading(
		float screenX,
		float screenY,
		float screenZ,
		float screenRadius,
		float frameRadius,
		color_t baseColor
	) :
		Shading(baseColor),
		m_screenX(screenX),
		m_screenY(screenY),
		m_screenZ(screenZ),
		m_screenRadius(screenRadius),
		m_frameRadius(frameRadius)
	{}

	virtual color_t operator()(int x, int y) const override;


private:
	float m_screenX;
	float m_screenY;
	float m_screenZ;
	float m_screenRadius;
	float m_frameRadius;
};
