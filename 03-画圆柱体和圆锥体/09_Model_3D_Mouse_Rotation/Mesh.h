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
	static void DivideTriangle(
		const point2& v0, const point2& v1, const point2& v2, 
		CMeshVertex *vbuf, int& vcounter, int depth);

	static void DivideTetra(
		const point3& v0, const point3& v1, 
		const point3& v2, const point3& v3,
		CMeshVertex *vbuf, int& vcounter, int depth);

	void CreateGLResources(
		CMeshVertex *vertices,
		GLuint *indices=NULL);

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

	void CreateGasket2D(
		const point2 triangle_vertices[3],
		int subdivision_depth);
	
	void CreateGasket3D(
		const point3 tetra_vertices[4],
		int subdivision_depth);
	
	void CreateCube(float size);

	void CreateRect(float sx, float sy, int nx, int ny);

	void CreateSphere(float radius, int num_slices, int num_stacks);

	void CreateCone(float radius, float h, int num_slices, int num_stacks, int rings);

	void CreateCylinder(float radius, float h, int num_slices, int num_stacks, int rings);
};

#endif
