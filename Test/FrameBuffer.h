#pragma once

#include <vector>
#include <mutex>
#include <map>


class Shading;
struct FrameRenderElement;


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
	void RenderSphere(const FrameRenderElement&);
	void RenderSphere2(const FrameRenderElement&);

	const color_t* GetFrameBuffer() const;
	int GetWidth() const { return m_iWidth; }
	int GetHeight() const { return m_iHeight; }

	bool IsCircleOnScene(float x, float y, float radius) const;


private:
	frameBuffer_t m_FramebufferArray;
	zBuffer_t m_ZBuffer;
	int m_iWidth;
	int m_iHeight;

	std::mutex mutex;

	//! Pixels of active circle.
	//! \see RenderSphere2()
	typedef std::pair< int, int > coord_t;
	typedef std::vector< coord_t > circle_t;
	typedef std::map< int /* sizeBox */, circle_t > poolCircles_t;
	//! Adaptive pool for fixed circle's radius.
	mutable poolCircles_t poolCircles;
};




struct FrameRenderElement {
	float screenX;
	float screenY;
	float screenZ;
	float screenRadius;
	CFrameBuffer::color_t ARGB;
};




//! \brief Base class for shading.
class Shading {
public:
	typedef CFrameBuffer::color_t color_t;


public:
	explicit Shading(const FrameRenderElement& fre) :
		m_fre(fre)
	{}

	virtual ~Shading()
	{}

	color_t GetBaseColor() const
	{
		return m_fre.ARGB;
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

	const FrameRenderElement& GetFrameRenderElement() const
	{
		return m_fre;
	}

	//! \param x Coord in a frame buffer.
	//! \param y Coord in a frame buffer.
	virtual color_t operator()(int x, int y) const = 0;


private:
	FrameRenderElement m_fre;
};




class DirectShading : public Shading {
public:
	explicit DirectShading(const FrameRenderElement& fre) :
		Shading(fre)
	{}

	virtual color_t operator()(int x, int y) const override;
};




class PhongShading : public Shading {
public:
	PhongShading(const FrameRenderElement& fre, float frameRadius) :
		Shading(fre),
		m_frameRadius(frameRadius)
	{}

	virtual color_t operator()(int x, int y) const override;


private:
	float m_frameRadius;
};
