#include "SphereData.h"
#include "FrameBuffer.h"
#include <stdio.h>
#include <math.h>
#include <algorithm>
#include <execution>


CSphereData::CSphereData(const char* szFilename)
{
	FILE* in;
	fopen_s(&in, szFilename, "rb");

	int num = 0;
	srand(1);
	for (;;)
	{
		SSphereElement el;

		if (fscanf_s(in, "%f %f %f", &el.x, &el.y, &el.z) != 3)
			break;

		el.y -= 60;
		el.z -= 50;

		el.x *= 0.01f;
		el.y *= 0.01f;
		el.z *= 0.01f;

		el.r = 5.0f + 5.0f * (rand() % 1024) / 1024.0f;
		el.r *= 0.004f;
		el.dwARGB = rand() & 0xff;
		el.dwARGB = (el.dwARGB << 8) | (rand() & 0xff);
		el.dwARGB = (el.dwARGB << 8) | (rand() & 0xff);

		m_SphereData.push_back(el);
		//if (num++ > 100) break;
	}

	fclose(in);
}


CSphereData::~CSphereData()
{
}


inline bool CompareSpheresFunc(
	const SSphereElement& s1,
	const SSphereElement& s2)
{
	return s1.screenZ > s2.screenZ;
}


void CSphereData::Render(CFrameBuffer& fb, float wi)
{
	const float s = sin(wi);
	const float c = cos(wi);

	std::for_each(
		std::execution::par,
		std::begin(m_SphereData),
		std::end(m_SphereData),
		[this, &fb, s, c](SSphereElement& ref) {
			ref.screenZ = ref.x * c + ref.z * s;
		});

	std::sort(
		std::execution::par,
		m_SphereData.begin(),
		m_SphereData.end(),
		CompareSpheresFunc);

	std::for_each(
		std::execution::par,
		std::begin(m_SphereData),
		std::end(m_SphereData),
		[this, &fb, s, c](const SSphereElement& ref) {
			const float fX = ref.x * s - ref.z * c;
			const float fY = ref.y;
			float fZ = ref.screenZ;
			fZ += 1.5f;
			if (fZ < 0.001f)
				return;

			const float fScreenX = fX / fZ;
			const float fScreenY = fY / fZ;
			const float fScreenZ = fZ;
			fb.RenderSphere(fScreenX, fScreenY, fScreenZ, ref.r / fZ, ref.dwARGB);
		});
}
