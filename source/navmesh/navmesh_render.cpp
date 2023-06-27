//#define _USE_MATH_DEFINES
//#include <math.h>
#include "mcv_platform.h"
#include "navmesh_render.h"
#include <stdio.h>
#include <stdarg.h>
#include "navmesh/recast/DetourDebugDraw.h"
#include <assert.h>
#include "render/primitives.h" 


class GLCheckerTexture
{
	unsigned int m_texId;
public:
	GLCheckerTexture() : m_texId(0)
	{
	}
	
	~GLCheckerTexture()
	{
		/*if (m_texId != 0)
			glDeleteTextures(1, &m_texId);*/
	}
	void bind()
	{
		//if (m_texId == 0)
		//{
		//	// Create checker pattern.
		//	const unsigned int col0 = duRGBA(215,215,215,255);
		//	const unsigned int col1 = duRGBA(255,255,255,255);
		//	static const int TSIZE = 64;
		//	unsigned int data[TSIZE*TSIZE];
		//	
		//	glGenTextures(1, &m_texId);
		//	glBindTexture(GL_TEXTURE_2D, m_texId);

		//	int level = 0;
		//	int size = TSIZE;
		//	while (size > 0)
		//	{
		//		for (int y = 0; y < size; ++y)
		//			for (int x = 0; x < size; ++x)
		//				data[x+y*size] = (x==0 || y==0) ? col0 : col1;
		//		glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, size,size, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		//		size /= 2;
		//		level++;
		//	}
		//	
		//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//}
		//else
		//{
		//	glBindTexture(GL_TEXTURE_2D, m_texId);
		//}
	}
};
GLCheckerTexture g_tex;


void DebugDrawGL::depthMask(bool state)
{
  /*if( state )
    Renderer.enableZWrite( );
  else
    Renderer.disableZWrite( );*/

}

void DebugDrawGL::texture( bool state )
{
	/*if (state)
	{
		glEnable(GL_TEXTURE_2D);
		g_tex.bind();
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
	}*/
}

void DebugDrawGL::begin(duDebugDrawPrimitives prim, float size)
{
  primitive = prim;
  primitive_size = size;
  num_colored_vertices = 0;
}

void DebugDrawGL::vertex(const float* pos, unsigned int color)
{
  assert( num_colored_vertices < MAX_VERTICES-1 );
  colored_vertices[ num_colored_vertices ].assign( pos[0], pos[1], pos[2], color );
  ++num_colored_vertices;
}

void DebugDrawGL::vertex(const float x, const float y, const float z, unsigned int color)
{
  assert( num_colored_vertices < MAX_VERTICES-1 );
  colored_vertices[ num_colored_vertices ].assign( x, y, z, color );
  ++num_colored_vertices;
}

void DebugDrawGL::vertex(const float* pos, unsigned int color, const float* uv)
{
  vertex( pos, color );
  /*assert( num_textured_colored_vertices < MAX_VERTICES-1 );
  textured_colored_vertices[ num_textured_colored_vertices ].assign( pos[0], pos[1], pos[2], color, uv[0], uv[1] );
  ++num_textured_colored_vertices;*/
}

void DebugDrawGL::vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v)
{
  vertex( x, y, z, color );
  /*assert( num_textured_colored_vertices < MAX_VERTICES - 1 );
  textured_colored_vertices[ num_textured_colored_vertices ].assign( x, y, z, color, u, v );
  ++num_textured_colored_vertices;*/
}

void DebugDrawGL::end()
{
	{
		for (int i = 0; i < num_colored_vertices - 2; i += 3) {
			drawLine(VEC3(colored_vertices[i].x, colored_vertices[i].y, colored_vertices[i].z),
				VEC3(colored_vertices[i + 1].x, colored_vertices[i + 1].y, colored_vertices[i + 1].z),
				VEC4(1, 0, 0, 1));

			drawLine(VEC3(colored_vertices[i + 1].x, colored_vertices[i + 1].y, colored_vertices[i + 1].z),
				VEC3(colored_vertices[i + 2].x, colored_vertices[i + 2].y, colored_vertices[i + 2].z),
				VEC4(1, 0, 0, 1));

			drawLine(VEC3(colored_vertices[i + 2].x, colored_vertices[i + 2].y, colored_vertices[i + 2].z),
				VEC3(colored_vertices[i].x, colored_vertices[i].y, colored_vertices[i].z),
				VEC4(1, 0, 0, 1));
		}

		num_colored_vertices = 0;
		num_textured_colored_vertices = 0;
	}
}

