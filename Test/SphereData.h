#pragma once

#include <vector>


struct SSphereElement
{
	float x, y, z, r, screenZ;
	unsigned int dwARGB;
};


class CFrameBuffer;


class CSphereData
{
public:
	CSphereData(const char* szFilename);
	~CSphereData();

	void Render(CFrameBuffer& fb, float wi);

	std::vector<SSphereElement> m_SphereData;
};
