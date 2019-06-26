#pragma once

#include <vector>


class CFrameBuffer
{
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
	std::vector<unsigned int> m_FramebufferArray;
	std::vector<unsigned int> m_ZBuffer;
	int m_iWidth;
	int m_iHeight;
};
