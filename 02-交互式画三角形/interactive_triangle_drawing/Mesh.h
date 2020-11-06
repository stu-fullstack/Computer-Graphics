#ifndef _MESH_H_
#define _MESH_H_

#include "GL/glew.h"
#include "vec.h"

class CMeshVertex
{
public:
	point3 pos;
	color4 color;
};

class CMesh
{
	void CreateGLResources(CMeshVertex *vertices);

public:
	GLuint vertex_array_obj;
	GLuint vertex_buffer_obj;
	int num_vertices;
	GLenum prmitive_type;

	CMesh(void);
	void ReleaseGLResources(void);

	void Draw(void);
	void CreateGenericModel(int num_vertices_in);

};

#endif
