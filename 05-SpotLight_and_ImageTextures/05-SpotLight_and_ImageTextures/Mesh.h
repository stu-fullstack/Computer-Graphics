#ifndef _MESH_H_
#define _MESH_H_

#include "GL/glew.h"
#include "vec.h"
#include "Camera.h"
// Mesh vertex
class CMeshVertex
{
public:
	point3 pos;    // Position
	color4 color;  // Color
	vec3 normal;   // Normal
	vec2 texcoord; // Texture coordinates
};

// Mesh
class CMesh
{
protected:
	static void DivideTriangle(
		const point2& v0, const point2& v1, const point2& v2, 
		CMeshVertex *vbuf, int& vcounter, int depth);
	// Recursive function that subdivides a triangle to create a 2D gasket
	// v0, v1, v2: (in) Triangle vertices
	// vbuf: (out) Vertex array
	// vcounter: (in and out) Vertex counter
	// depth: (in) Recursion depth

	static void DivideTetra(
		const point3& v0, const point3& v1, 
		const point3& v2, const point3& v3,
		CMeshVertex *vbuf, int& vcounter, int depth);
	// Recursive function that subdivides a tetrahedron to create a 3D gasket
	// v0, v1, v2, v3: (in) Tetrahedron vertices
	// vbuf: (out) Vertex array
	// vcounter: (in and out) Vertex counter
	// depth: (in) Recursion depth

	void CreateGLResources(
		CMeshVertex *vertices,
		GLuint *indices=NULL);	void CreateGLResources2(
		CMeshVertex *vertices,
		GLuint *indices=NULL);
	// Create OpenGL resources
	// vertices: (in) Vertex array
	// indices: (in) Index array
	//          NULL indicates that the mesh does not have an index array
	// Input member variables:
	//     num_vertices, num_indices
	// Output member variables:
	//     vertex_array_obj, vertex_buffer_obj, index_buffer_obj

public:
	GLuint vertex_array_obj;  // OpenGL vertex array object
	GLuint vertex_buffer_obj; // OpenGL vertex buffer object
	GLuint index_buffer_obj;  // OpenGL index buffer object
	int num_vertices; // The number of vertices
	int num_indices;  // The number of indices
	GLenum primitive_type; // OpenGL primitive type

	CMesh(void);

	void ReleaseGLResources(void);
	// Release OpenGL resources

	void Draw(void);
	// Draw the mesh

	void CreateGasket2D(
		const point2 triangle_vertices[3],
		int subdivision_depth);
	// Create a 2D gasket
	// triangle_vertices: (in) Triangle vertices
	// subdivision_depth: (in) Maximum recursive subdivision depth

	void CreateGasket3D(
		const point3 tetra_vertices[4],
		int subdivision_depth);
	// Create a 3D gasket
	// tetra_vertices:    (in) Tetrahedron vertices
	// subdivision_depth: (in) Maximum recursive subdivision depth
	
	void CreateCube(float size, float ntex);
	// Create a cube
	// size: (in) Cube edge length
	// ntex: (in) Texture coordinate multiplier in x, y, z directions

	void CreateBlock(float sx, float sy, float sz,
		float tex_nx, float tex_ny, float tex_nz);
	// Create a block
	// sx, sy, sz: (in) Block sizes in x, y, z directions
	// tex_nx, tex_ny, tex_nz: (in) Texture coordinate multipliers in x, y, z directions

	void CreateRect(float sx, float sy, int nx, int ny,
		float tex_nx, float tex_ny);
	// Create a rectangle
	// sx, sy: (in) Rectangle sizes in x, y directions
	// nx, ny: (in) The numbers of subdivisions in x, y directions
	// tex_nx, tex_ny: (in) Texture coordinate multipliers in x, y directions

	void CreateSphere(float radius, int num_slices, int num_stacks,
		float tex_ntheta, float tex_nphi);
	// Create a sphere
	// radius: (in) Sphere radius
	// num_slices: (in) The number of subdivisions in theta direction
	// num_stacks: (in) The number of subdivisions in phi direction
	// tex_ntheta: (in) Texture coordinate multiplier in theta direction
	// tex_nphi:   (in) Texture coordinate multiplier in phi direction

	void CreateAxes(float sx, float sy, float sz);

	void CreateCone(float radius, float h, int num_slices, int num_stacks, int rings, float tex_ntheta, float tex_nh);

	void CreateCylinder(float radius, float h, int num_slices, int num_stacks, int rings, float tex_nx, float tex_ny);

};

#endif
