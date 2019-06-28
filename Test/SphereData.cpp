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
		el.sphere = new SSphere;
		if (fscanf_s(in, "%f %f %f",
			&el.sphere->x, &el.sphere->y, &el.sphere->z) != 3)
		{
			break;
		}

		el.sphere->y -= 60;
		el.sphere->z -= 50;

		el.sphere->x *= 0.01f;
		el.sphere->y *= 0.01f;
		el.sphere->z *= 0.01f;

		el.sphere->r = 5.0f + 5.0f * (rand() % 1024) / 1024.0f;
		el.sphere->r *= 0.004f;

		el.sphere->dwARGB = rand() & 0xff;
		el.sphere->dwARGB = (el.sphere->dwARGB << 8) | (rand() & 0xff);
		el.sphere->dwARGB = (el.sphere->dwARGB << 8) | (rand() & 0xff);

		/* test
		if (num == 0) {
			// red
			el.sphere->dwARGB = 0xFFFF0000;
		}
		else if (num == 1) {
			// green
			el.sphere->dwARGB = 0xFF00FF00;
		}
		else if (num == 2) {
			// blue
			el.sphere->dwARGB = 0xFF0000FF;
		}
		*/

		m_SphereData.push_back(el);

		++num;
		//if (num > 100) break;
	}

	fclose(in);
}


CSphereData::~CSphereData()
{
	for (auto&& el : m_SphereData) {
		delete el.sphere;
	}
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
			ref.screenZ = ref.sphere->z * s + ref.sphere->x * c;
		});

	std::sort(
		std::execution::par,
		m_SphereData.begin(),
		m_SphereData.end(),
		[](const SSphereElement& s1, const SSphereElement& s2)
		{
			return s1.screenZ < s2.screenZ;
		});

	std::for_each(
		std::execution::par,
		std::begin(m_SphereData),
		std::end(m_SphereData),
		[this, &fb, s, c](const SSphereElement& ref) {
			const float fX = ref.sphere->x * s - ref.sphere->z * c;
			const float fY = ref.sphere->y;
			float fZ = ref.screenZ;
			fZ += 1.5f;
			if (fZ < 0.001f)
				return;

			const FrameRenderElement fre{
				fX / fZ,
				fY / fZ,
				fZ,
				ref.sphere->r / fZ,
				ref.sphere->dwARGB
			};
			fb.RenderSphere2(fre);
		});

	// pause
	//std::this_thread::sleep_for(std::chrono::milliseconds(2000));
}
