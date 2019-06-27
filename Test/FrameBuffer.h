#pragma once

#include <vector>
#include <mutex>


class CFrameBuffer
{
public:
	typedef std::vector< unsigned int > frameBuffer_t;
	typedef std::vector< float > zBuffer_t;


public:
	CFrameBuffer(const int iWidth, const int iHeight);

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
		unsigned int ARGB);

	const unsigned int* GetFrameBuffer() const;
	int GetWidth() const { return m_iWidth; }
	int GetHeight() const { return m_iHeight; }


private:
	frameBuffer_t m_FramebufferArray;
	zBuffer_t m_ZBuffer;
	int m_iWidth;
	int m_iHeight;

	std::mutex mutex;

	friend class CSphereData;
};
