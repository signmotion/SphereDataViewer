#include "FrameBuffer.h"
#include "SphereData.h"

#include <stdio.h>
#include <math.h>
#include <algorithm>

CSphereData::CSphereData( const char *szFilename )
{
	FILE *in;
	fopen_s(&in,szFilename,"rb");

	int num = 0;
	srand(1);
	for(;;)
	{
		SSphereElement el;

		if(fscanf_s(in,"%f %f %f",&el.x,&el.y,&el.z)!=3)
			break;

		el.y-=60;
		el.z-=50;

		el.x*=0.01f;
		el.y*=0.01f;
		el.z*=0.01f;

		el.r = 5.0f + 5.0f * (rand()%1024)/1024.0f;
		el.r *= 0.004f;
		el.dwARGB = rand()&0xff;
		el.dwARGB = (el.dwARGB<<8) | (rand()&0xff);
		el.dwARGB = (el.dwARGB<<8) | (rand()&0xff);

		m_SphereData.push_back(el);
		//if (num++ > 100) break;
	}

	fclose(in);
}

CSphereData::~CSphereData()
{
}

inline bool CompareSpheresFunc( const SSphereElement &s1,const SSphereElement &s2 )
{
	return s1.screenZ > s2.screenZ;
}

void CSphereData::Render( CFrameBuffer &fb, float wi )
{
	std::vector<SSphereElement>::iterator it, end=m_SphereData.end();

	float s=sin(wi);
	float c=cos(wi);

	for(it=m_SphereData.begin();it!=end;++it)
	{
		SSphereElement &ref = *it;
		ref.screenZ = ref.x*c+ref.z*s;
	}

	std::sort( m_SphereData.begin(),m_SphereData.end(),CompareSpheresFunc );

	for(it=m_SphereData.begin();it!=end;++it)
	{
		const SSphereElement &ref = *it;

		float fX = ref.x*s-ref.z*c;
		float fY = ref.y;
		//float fZ = ref.x*c+ref.z*s;
		float fZ = ref.screenZ;

		fZ += 1.5f;

		if(fZ<0.001f)
			continue;

		float fScreenX = fX/fZ;
		float fScreenY = fY/fZ;
		float fScreenZ = fZ;

		fb.RenderSphere(fScreenX,fScreenY,fScreenZ,ref.r/fZ,ref.dwARGB);
	}
}