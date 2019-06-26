#pragma once

#include <vector>


struct SSphere
{
	float x, y, z, r;
	unsigned int dwARGB;
};


struct SSphereElement
{
	float screenZ;
	SSphere* sphere;
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
