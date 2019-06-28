#pragma once

#include <vector>


__declspec(align(32)) struct SSphere
{
	float x, y, z, r;
	unsigned int dwARGB;
};


__declspec(align(32)) struct SSphereElement
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

private:
	std::vector<SSphereElement> m_SphereData;
};
