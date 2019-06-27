#pragma once

#include <vector>
#include <mutex>


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
