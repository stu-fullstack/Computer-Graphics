#ifndef _MESH_H_
#define _MESH_H_

#include "GL/glew.h"
#include "vec.h"

class CMeshVertex
{
public:
	point2 pos;
	color4 color;
};

class CMesh
{
protected:
	static void DivideLine(
		const point2& p0, const point2& p1, 
		CMeshVertex *vbuf, int& vcounter, int depth);
	static point2 CalNewPoint(point2 v0, point2 v1);
public:
	GLuint vertex_array_obj;
	GLuint vertex_buffer_obj;
	int num_vertices;
	GLenum prmitive_type;

	CMesh(void);
	void ReleaseGLResources(void);

	void Draw(void);
	void CreateKochSnowflate(
		const point2 snow_vertices[3],
		int subdivision_depth);
};

#endif
