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
protected:
	void CreateGLResources(CMeshVertex* vertices, GLuint* indices = NULL);

public:
	GLuint vertex_array_obj;
	GLuint vertex_buffer_obj;
	GLuint index_buffer_obj;
	int num_vertices;
	int num_indices;
	GLenum prmitive_type;

	CMesh(void);

	void ReleaseGLResources(void);
	void Draw(void);

	void CreateCone(float radius, float h, int num_slices, int num_stacks, int num_rings);
};

#endif
