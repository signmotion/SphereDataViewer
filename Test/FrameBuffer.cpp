
#include "FrameBuffer.h"
#include <math.h>
#include <algorithm>

struct Vec3
{
	double x,y,z;

	Vec3 normalize()
	{
		double d = sqrt(x*x + y*y + z*z);
		Vec3 vec;
		vec.x = x / d;
		vec.y = y / d;
		vec.z = z / d;
		return vec;
	}
	double dot( Vec3 v2 )
	{
		return x*v2.x + y*v2.y + z*v2.z;
	}
};

// Global light
Vec3 Light;

//////////////////////////////////////////////////////////////////////////

CFrameBuffer::CFrameBuffer( const int iWidth, const int iHeight )
	:m_iWidth(iWidth), m_iHeight(iHeight)
{
	m_FramebufferArray.resize(iWidth*iHeight,0);

	Light.x = 1.f;
	Light.y = -0.5f;
	Light.z = 0.7f;
	Light = Light.normalize();
}

CFrameBuffer::~CFrameBuffer()
{
}

void CFrameBuffer::Clear()
{
	memset(&m_FramebufferArray[0],0,m_iWidth*m_iHeight*4);
	
	//Light.x -= 0.02f; // some light animation
	//Light = Light.normalize();
}
	
const unsigned int* CFrameBuffer::GetFrameBuffer() const
{
	return static_cast<const unsigned int*>(&m_FramebufferArray[0]);
};

void CFrameBuffer::RenderSphere( float fScreenX, float fScreenY, float fScreenZ, float fScreenRadius, unsigned int ARGB )
{
	float centerX = fScreenX*m_iWidth/2 + m_iWidth/2;
	float centerY = fScreenY*m_iWidth/2 + m_iWidth/2;

	float RX = fScreenRadius*m_iWidth/2;
	float RY = fScreenRadius*m_iWidth/2;

	for(int x = centerX-RX*2; x <= centerX+RX*2;++x)
	{
		for(int y = centerY-RY*2; y <= centerY+RY*2;++y)
		{
			int dx = x-centerX;
			int dy = y-centerY;

			if (dx*dx + dy*dy > RX*RY)
				continue;
			if(y<0 || y>=m_iHeight)
				continue;
			if(x<0 || x>=m_iWidth)
				continue;
	
			// Phong shading
			{
				Vec3 vec_normal;
				vec_normal.x = (float)dx;
				vec_normal.y = (float)dy;
				vec_normal.z = sqrt(float(RX*RY) - (dx*dx + dy*dy));
				vec_normal = vec_normal.normalize();

				float NdotL = Light.dot(vec_normal);
				if (NdotL > 0)
				{
					Vec3 vec_eye;
					vec_eye.x = Light.x+fScreenX;
					vec_eye.y = Light.y+fScreenY;
					vec_eye.z = Light.z+1.0f;
					Vec3 vec_half = vec_eye.normalize();

					float NdotHV = vec_half.dot(vec_normal);
					float specular = pow(NdotHV,9); // shininess=9
					float alpha = (NdotL+specular);
					if (alpha > 1.0f)
						alpha = 1.0f;

					float r = ((ARGB&0xFF0000)>>16)*alpha;
					float g = ((ARGB&0x00FF00)>>8)*alpha;
					float b = ((ARGB&0x0000FF))*alpha;
					r = std::min(r,255.f);
					g = std::min(g,255.f);
					b = std::min(b,255.f);

					m_FramebufferArray[x+y*m_iWidth]  = int(r)<<16;
					m_FramebufferArray[x+y*m_iWidth] |= int(g)<<8;
					m_FramebufferArray[x+y*m_iWidth] |= int(b)<<0;
				}
			}
		}
	}
}
