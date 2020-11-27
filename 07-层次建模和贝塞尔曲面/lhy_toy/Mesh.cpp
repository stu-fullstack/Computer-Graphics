#include <stddef.h>
#include "Mesh.h"

#include <fstream>
#include <iostream>
using namespace std;

void CMesh::DivideTriangle(
	const point2& v0, const point2& v1, const point2& v2, 
	CMeshVertex *vbuf, int& vcounter, int depth)
// Recursive function that subdivides a triangle to create a 2D gasket
// v0, v1, v2: (in) Triangle vertices
// vbuf: (out) Vertex array
// vcounter: (in and out) Vertex counter
// depth: (in) Recursion depth
{
	if (depth>0)
	{
		point2 t0=0.5f*(v1+v2);
		point2 t1=0.5f*(v2+v0);
		point2 t2=0.5f*(v0+v1);

		DivideTriangle(v0, t2, t1, vbuf, vcounter, depth-1);
		DivideTriangle(v1, t0, t2, vbuf, vcounter, depth-1);
		DivideTriangle(v2, t1, t0, vbuf, vcounter, depth-1);
	}
	else
	{
		vbuf[vcounter].color=color4(1.0f, 0.0f, 0.0f, 1.0f);
		vbuf[vcounter++].pos=vec3(v0, 0.0f);
		vbuf[vcounter].color=color4(0.0f, 1.0f, 0.0f, 1.0f);
		vbuf[vcounter++].pos=vec3(v1, 0.0f);
		vbuf[vcounter].color=color4(0.0f, 0.0f, 1.0f, 1.0f);
		vbuf[vcounter++].pos=vec3(v2, 0.0f);
	}
}

void CMesh::DivideTetra(
	const point3& v0, const point3& v1, 
	const point3& v2, const point3& v3,
	CMeshVertex *vbuf, int& vcounter, int depth)
// Recursive function that subdivides a tetrahedron to create a 3D gasket
// v0, v1, v2, v3: (in) Tetrahedron vertices
// vbuf: (out) Vertex array
// vcounter: (in and out) Vertex counter
// depth: (in) Recursion depth
{
	const static color4 base_colors[4]={
		color4(1.0f, 0.0f, 0.0f, 1.0f),
		color4(0.0f, 1.0f, 0.0f, 1.0f),
		color4(0.0f, 0.0f, 1.0f, 1.0f),
		color4(0.3f, 0.3f, 0.3f, 1.0f),
	};

	if (depth>0)
	{
		point3 m[6];
		m[0]=0.5f*(v1+v2);
		m[1]=0.5f*(v2+v0);
		m[2]=0.5f*(v0+v1);
		m[3]=0.5f*(v3+v0);
		m[4]=0.5f*(v3+v1);
		m[5]=0.5f*(v3+v2);

		DivideTetra(v0, m[2], m[1], m[3], 
			vbuf, vcounter, depth-1);
		DivideTetra(m[2], v1, m[0], m[4], 
			vbuf, vcounter, depth-1);
		DivideTetra(m[1], m[0], v2, m[5], 
			vbuf, vcounter, depth-1);
		DivideTetra(m[3], m[4], m[5], v3, 
			vbuf, vcounter, depth-1);
	}
	else
	{
		const point3 *pv[4]={&v0, &v1, &v2, &v3};
		const static int iv_face[4][3]={
			{3,1,2}, {3,2,0}, {3,0,1}, {0,2,1}
		};
		vec3 N;
		for (int i=0; i<4; ++i)
		{
			N=TriangleNormal(
				*pv[iv_face[i][0]],
				*pv[iv_face[i][1]],
				*pv[iv_face[i][2]]
				);
			vbuf[vcounter].color=base_colors[i];
			vbuf[vcounter].normal=N;
			vbuf[vcounter++].pos=*pv[iv_face[i][0]];
			vbuf[vcounter].color=base_colors[i];
			vbuf[vcounter].normal=N;
			vbuf[vcounter++].pos=*pv[iv_face[i][1]];
			vbuf[vcounter].color=base_colors[i];
			vbuf[vcounter].normal=N;
			vbuf[vcounter++].pos=*pv[iv_face[i][2]];
		}
	}
}

CMesh::CMesh(void)
{
	primitive_type=GL_TRIANGLES;
	num_vertices=0;
	num_indices=0;
	vertex_array_obj=0;
	vertex_buffer_obj=0;
	index_buffer_obj=0;
}

void CMesh::ReleaseGLResources(void)
// Release OpenGL resources
{
	if (vertex_array_obj!=0)
		glDeleteVertexArrays(1, &vertex_array_obj);
	vertex_array_obj=0;

	if (vertex_buffer_obj!=0)
		glDeleteBuffers(1, &vertex_buffer_obj);
	vertex_buffer_obj=0;

	if (index_buffer_obj!=0)
		glDeleteBuffers(1, &index_buffer_obj);
	index_buffer_obj=0;
}

void CMesh::Draw(void)
// Draw the mesh
{
	glBindVertexArray(vertex_array_obj);
	if (index_buffer_obj==0)
		glDrawArrays(primitive_type, 0, num_vertices);
	else
		glDrawElements(primitive_type, num_indices, 
			GL_UNSIGNED_INT, (GLvoid *)0);
	glBindVertexArray(0);
}

void CMesh::CreateGLResources(
	CMeshVertex *vertices, GLuint *indices)
// Create OpenGL resources
// vertices: (in) Vertex array
// indices: (in) Index array
//          NULL indicates that the mesh does not have an index array
// Input member variables:
//     num_vertices, num_indices
// Output member variables:
//     vertex_array_obj, vertex_buffer_obj, index_buffer_obj
{
	// Create the vertex array object
	glGenVertexArrays(1, &vertex_array_obj);
	glBindVertexArray(vertex_array_obj);

	// Create the vertex buffer object
	glGenBuffers(1, &vertex_buffer_obj);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_obj);
	glBufferData(GL_ARRAY_BUFFER, 
		sizeof(CMeshVertex)*num_vertices,
		vertices, GL_STATIC_DRAW);

	// Enable vertex attribute arrays and
	//   set their data and formats in the vertex buffer object
	glEnableVertexAttribArray(0); // 0=position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 
		sizeof(CMeshVertex), (GLvoid *)offsetof(CMeshVertex, pos));

	glEnableVertexAttribArray(1); // 1=color
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 
		sizeof(CMeshVertex), (GLvoid *)offsetof(CMeshVertex, color));

	glEnableVertexAttribArray(2); // 2=normal
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 
		sizeof(CMeshVertex), (GLvoid *)offsetof(CMeshVertex, normal));

	glEnableVertexAttribArray(3); // 3=texture coordinates
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 
		sizeof(CMeshVertex), (GLvoid *)offsetof(CMeshVertex, texcoord));

	// Create the index buffer object if necessary
	if (indices!=NULL)
	{
		glGenBuffers(1, &index_buffer_obj);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_obj);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
			sizeof(GLuint)*num_indices,
			indices, GL_STATIC_DRAW);
	}

	// Restore the current vertex array object to the default one
	glBindVertexArray(0);


}

void CMesh::CreateGasket2D(
	const point2 triangle_vertices[3],
	int subdivision_depth)
// Create a 2D gasket
// triangle_vertices: (in) Triangle vertices
// subdivision_depth: (in) Maximum recursive subdivision depth
{
	num_vertices=1;
	int i;
	for (i=0; i<=subdivision_depth; ++i)
		num_vertices*=3;
	CMeshVertex *vertices=new CMeshVertex [num_vertices];

	i=0;
	DivideTriangle(
		triangle_vertices[0],
		triangle_vertices[1],
		triangle_vertices[2],
		vertices, 
		i, subdivision_depth);

	CreateGLResources(vertices);

	delete [] vertices;
}

void CMesh::CreateGasket3D(
	const point3 tetra_vertices[4],
	int subdivision_depth)
// Create a 3D gasket
// tetra_vertices:    (in) Tetrahedron vertices
// subdivision_depth: (in) Maximum recursive subdivision depth
{
	num_vertices=3;
	int i;
	for (i=0; i<=subdivision_depth; ++i)
		num_vertices*=4;
	CMeshVertex *vertices=new CMeshVertex [num_vertices];

	i=0;
	DivideTetra(
		tetra_vertices[0],
		tetra_vertices[1],
		tetra_vertices[2],
		tetra_vertices[3],
		vertices, 
		i, subdivision_depth);

	CreateGLResources(vertices);

	delete [] vertices;
}

void CMesh::CreateCube(float size, float ntex)
// Create a cube
// size: (in) Cube edge length
// ntex: (in) Texture coordinate multiplier in x, y, z directions
{
	// Normalized positions of 8 vertices
	static const point3 vpos[8]={
		point3(-1.0f, -1.0f, -1.0f),
		point3(1.0f, -1.0f, -1.0f),
		point3(1.0f, 1.0f, -1.0f),
		point3(-1.0f, 1.0f, -1.0f),
		point3(-1.0f, -1.0f, 1.0f),
		point3(1.0f, -1.0f, 1.0f),
		point3(1.0f, 1.0f, 1.0f),
		point3(-1.0f, 1.0f, 1.0f)
	};

	// Normal vectors of 6 faces
	static const vec3 nface[6]={
		vec3(1.0f, 0.0f, 0.0f),
		vec3(-1.0f, 0.0f, 0.0f),
		vec3(0.0f, 1.0f, 0.0f),
		vec3(0.0f, -1.0f, 0.0f),
		vec3(0.0f, 0.0f, 1.0f),
		vec3(0.0f, 0.0f, -1.0f)
	};

	// Vertex indices of 6 faces
	static const int iv_face[6][4]={
		{1, 2, 6, 5},
		{3, 0, 4, 7},
		{2, 3, 7, 6},
		{0, 1, 5, 4},
		{4, 5, 6, 7},
		{1, 0, 3, 2}
	};

	// Texture coordinates of 4 vertices of a single face
	static const float tc_face[4][2]={
		{0, 0}, {1, 0}, {1, 1}, {0, 1}
	};

	// Set the number of vertices and indices
	num_vertices=24;
	num_indices=36;

	// Allocate memory for vertices and indices
	CMeshVertex *vertices=new CMeshVertex [num_vertices];
	GLuint *indices=new GLuint [num_indices];

	// Loop through all 6 faces
	int i, k;
	float sh=0.5f*size;
	for (i=0; i<6; ++i)
	{
		int iv0=i*4;
		
		// Set parameters for the 4 vertices of current face
		int counter=iv0;
		for (k=0; k<4; ++k, ++counter)
		{
			// Set vertex position
			vertices[counter].pos=vpos[iv_face[i][k]]*sh;

			// Set vertex normal
			vertices[counter].normal=nface[i];

			// Set vertex texture coordinates
			vertices[counter].texcoord.x=tc_face[k][0]*ntex;
			vertices[counter].texcoord.y=tc_face[k][1]*ntex;

			// Set vertex color
			vertices[counter].color=color4(1.0f, 1.0f, 1.0f, 1.0f);
		}

		// Set vertex indices of the two triangles of current face
		k=i*6;
		indices[k++]=iv0;
		indices[k++]=iv0+1;
		indices[k++]=iv0+2;
		indices[k++]=iv0;
		indices[k++]=iv0+2;
		indices[k++]=iv0+3;
	}

	// Create OpenGL resources
	CreateGLResources(vertices, indices);

	// Free memory
	delete [] vertices;
	delete [] indices;
}

void CMesh::CreateBlock(float sx, float sy, float sz,
	float tex_nx, float tex_ny, float tex_nz)
// Create a block
// sx, sy, sz: (in) Block sizes in x, y, z directions
// tex_nx, tex_ny, tex_nz: (in) Texture coordinate multipliers in x, y, z directions
{
	// Normalized positions of 8 vertices
	static const point3 vpos[8]={
		point3(-1.0f, -1.0f, -1.0f),
		point3(1.0f, -1.0f, -1.0f),
		point3(1.0f, 1.0f, -1.0f),
		point3(-1.0f, 1.0f, -1.0f),
		point3(-1.0f, -1.0f, 1.0f),
		point3(1.0f, -1.0f, 1.0f),
		point3(1.0f, 1.0f, 1.0f),
		point3(-1.0f, 1.0f, 1.0f)
	};

	// Normal vectors of 6 faces
	static const vec3 nface[6]={
		vec3(1.0f, 0.0f, 0.0f),
		vec3(-1.0f, 0.0f, 0.0f),
		vec3(0.0f, 1.0f, 0.0f),
		vec3(0.0f, -1.0f, 0.0f),
		vec3(0.0f, 0.0f, 1.0f),
		vec3(0.0f, 0.0f, -1.0f)
	};

	// Vertex indices of 6 faces
	static const int iv_face[6][4]={
		{1, 2, 6, 5},
		{3, 0, 4, 7},
		{2, 3, 7, 6},
		{0, 1, 5, 4},
		{4, 5, 6, 7},
		{1, 0, 3, 2}
	};

	// Texture coordinates of 4 vertices of a single face
	static const float tc_face[4][2]={
		{0, 0}, {1, 0}, {1, 1}, {0, 1}
	};

	// Local x, y directions of 6 faces (0=x, 1=y, 2=z)
	static const int face_local_axes[6][2]={
		{1, 2}, {1, 2}, {0, 2}, {0, 2}, {0, 1}, {0, 1}
	};

	// Texture coordinate multipliers in x, y, z directions
	float axes_ntex[3]={tex_nx, tex_ny, tex_nz};

	// Set the number of vertices and indices
	num_vertices=24;
	num_indices=36;

	// Allocate memory for vertices and indices
	CMeshVertex *vertices=new CMeshVertex [num_vertices];
	GLuint *indices=new GLuint [num_indices];

	// Loop through all 6 faces
	int i, k;
	float sx_h=0.5f*sx;
	float sy_h=0.5f*sy;
	float sz_h=0.5f*sz;
	for (i=0; i<6; ++i)
	{
		int iv0=i*4;
		
		// Set parameters for the 4 vertices of current face
		int counter=iv0;
		for (k=0; k<4; ++k, ++counter)
		{
			// Set vertex position
			vertices[counter].pos.x=vpos[iv_face[i][k]].x*sx_h;
			vertices[counter].pos.y=vpos[iv_face[i][k]].y*sy_h;
			vertices[counter].pos.z=vpos[iv_face[i][k]].z*sz_h;

			// Set vertex normal
			vertices[counter].normal=nface[i];

			// Set vertex texture coordinates
			vertices[counter].texcoord.x=tc_face[k][0]*axes_ntex[face_local_axes[i][0]];
			vertices[counter].texcoord.y=tc_face[k][1]*axes_ntex[face_local_axes[i][1]];

			// Set vertex color
			vertices[counter].color=color4(1.0f, 1.0f, 1.0f, 1.0f);
		}

		// Set vertex indices of the two triangles of current face
		k=i*6;
		indices[k++]=iv0;
		indices[k++]=iv0+1;
		indices[k++]=iv0+2;
		indices[k++]=iv0;
		indices[k++]=iv0+2;
		indices[k++]=iv0+3;
	}

	// Create OpenGL resources
	CreateGLResources(vertices, indices);

	// Free memory
	delete [] vertices;
	delete [] indices;
}

void CMesh::CreateRect(
	float sx, float sy, int nx, int ny, float tex_nx, float tex_ny)
// Create a rectangle
// sx, sy: (in) Rectangle sizes in x, y directions
// nx, ny: (in) The numbers of subdivisions in x, y directions
// tex_nx, tex_ny: (in) Texture coordinate multipliers in x, y directions
{
	float dx=sx/nx;
	float dy=sy/ny;
	float sx_h=0.5f*sx;
	float sy_h=0.5f*sy;

	// Set the number of vertices and indices
	num_vertices=(nx+1)*(ny+1);
	num_indices=6*nx*ny;

	// Allocate memory for vertices and indices
	CMeshVertex *vertices=new CMeshVertex [num_vertices];
	GLuint *indices=new GLuint [num_indices];

	// Set vertices
	int i, j, k;
	for (j=0, k=0; j<=ny; ++j) // Loop through all rows in y direction
	{
		float y=-sy_h+dy*j;
		float texcoord_t=(float)j/(float)ny *tex_ny;
		for (i=0; i<=nx; ++i, ++k) // Loop through all columns in x direction
		{
			// Set vertex position
			float x=-sx_h+dx*i;
			vertices[k].pos.x=x;
			vertices[k].pos.y=y;
			vertices[k].pos.z=0.0f;

			// Set vertex normal
			vertices[k].normal=vec3(0.0f, 0.0f, 1.0f);

			// Set vertex texture coordinates
			vertices[k].texcoord.x=(float)i/(float)nx *tex_nx;
			vertices[k].texcoord.y=texcoord_t;

			// Set vertex color
			vertices[k].color=vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}

	// Set indices
	int iv[4];
	for (j=0, k=0; j<ny; ++j) // Loop through all rows in y direction
	{
		int ibase=j*(nx+1);
		for (i=0; i<nx; ++i) // Loop through all columns in x direction
		{
			// Calculate indices of the 4 vertices of current sub rectangle
			iv[0]=ibase+i;
			iv[1]=iv[0]+1;
			iv[3]=iv[0]+nx+1;
			iv[2]=iv[3]+1;

			// Subdivide current sub rectangle into two triangles
			//   in checkerboard fashion and set vertex indices of the two triangles
			if (i%2==j%2)
			{
				indices[k++]=iv[0];
				indices[k++]=iv[1];
				indices[k++]=iv[2];
				indices[k++]=iv[0];
				indices[k++]=iv[2];
				indices[k++]=iv[3];
			}
			else
			{
				indices[k++]=iv[0];
				indices[k++]=iv[1];
				indices[k++]=iv[3];
				indices[k++]=iv[1];
				indices[k++]=iv[2];
				indices[k++]=iv[3];
			}
		}
	}

	// Create OpenGL resources
	CreateGLResources(vertices, indices);

	// Free memory
	delete [] vertices;
	delete [] indices;
}

void CMesh::CreateSphere(
	float radius, int num_slices, int num_stacks,
	float tex_ntheta, float tex_nphi)
// Create a sphere
// radius: (in) Sphere radius
// num_slices: (in) The number of subdivisions in theta direction
// num_stacks: (in) The number of subdivisions in phi direction
// tex_ntheta: (in) Texture coordinate multiplier in theta direction
// tex_nphi:   (in) Texture coordinate multiplier in phi direction
{
	float dtheta=(M_PI+M_PI)/num_slices;
	float dphi=M_PI/num_stacks;

	// Set the number of vertices and indices
	num_vertices=(num_slices+1)*(num_stacks+1);
	num_indices=6*num_slices*num_stacks;

	// Allocate memory for vertices and indices
	CMeshVertex *vertices=new CMeshVertex [num_vertices];
	GLuint *indices=new GLuint [num_indices];

	// Set vertices
	int i, j, k;
	for (j=0, k=0; j<=num_stacks; ++j) // Loop through all rows in phi direction
	{
		float phi=-0.5*M_PI+dphi*j;
		float cphi=cos(phi);
		float sphi=sin(phi);
		float texcoord_t=(float)j/(float)num_stacks*tex_nphi;
		for (i=0; i<=num_slices; ++i, ++k) // Loop through all columns in theta direction
		{
			float theta=dtheta*i;
			float ctheta=cos(theta);
			float stheta=sin(theta);

			// Set vertex normal
			vertices[k].normal.x=cphi*ctheta;
			vertices[k].normal.y=cphi*stheta;
			vertices[k].normal.z=sphi;

			// Set vertex position
			vertices[k].pos=radius*vertices[k].normal;

			// Set vertex texture coordinates
			vertices[k].texcoord.x=(float)i/(float)num_slices*tex_ntheta;
			vertices[k].texcoord.y=texcoord_t;

			// Set vertex color
			vertices[k].color=vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}

	// Set indices
	int iv[4];
	for (j=0, k=0; j<num_stacks; ++j) // Loop through all rows in phi direction
	{
		int ibase=j*(num_slices+1);
		for (i=0; i<num_slices; ++i) // Loop through all columns in theta direction
		{
			// Calculate indices of the 4 vertices of current sub rectangle in parametric space
			iv[0]=ibase+i;
			iv[1]=iv[0]+1;
			iv[3]=iv[0]+num_slices+1;
			iv[2]=iv[3]+1;

			// Subdivide current sub rectangle in parametric space into two triangles
			//   in checkerboard fashion and set vertex indices of the two triangles
			if (i%2==j%2)
			{
				indices[k++]=iv[0];
				indices[k++]=iv[1];
				indices[k++]=iv[2];
				indices[k++]=iv[0];
				indices[k++]=iv[2];
				indices[k++]=iv[3];
			}
			else
			{
				indices[k++]=iv[0];
				indices[k++]=iv[1];
				indices[k++]=iv[3];
				indices[k++]=iv[1];
				indices[k++]=iv[2];
				indices[k++]=iv[3];
			}
		}
	}

	// Create OpenGL resources
	CreateGLResources(vertices, indices);

	// Free memory
	delete [] vertices;
	delete [] indices;
}

float Bernstein(int i, float u) {
	const vec4 bc = vec4(1, 3, 3, 1);
	return bc[i] * pow(u, i) * pow(1.0 - u, 3 - i);
}
void CMesh::DivideBezierPatch(int& counter, int& iv_counter, int patch_index, CMeshVertex* vbuf, GLuint* indices, point3* cp_vertices, int cp_indices[16], float increment, float tex_u, float tex_v)
{
	int i, j;
	int u_cnt = 0, v_cnt = 0;							//u,v����ϸ�ֵļ�����
	int u_num = (1 / increment) + 1, v_num = u_num;		//u,vϸ�ֺ�Ķ�������
	float u = 0.0, v = 0.0;								//Bernstein �е� u,v����
	vec4 curve_p = vec4(0.0);							//�����ϵĵ㣨�м������

	while (u_cnt < u_num)
	{
		while (v_cnt < v_num)
		{
			for (i = 0; i < 4; i++)		//���������ϵĵ� p(u, v)
			{
				for (j = 0; j < 4; j++)
				{
					curve_p += Bernstein(i, u) * Bernstein(j, v) * cp_vertices[cp_indices[i * 4 + j]];
				}
			}

			vbuf[counter].color = color4(1.0, 1.0, 1.0, 1.0);
			vbuf[counter].texcoord.x = 1.0 * v_cnt / v_num * tex_v;
			vbuf[counter].texcoord.y = 1.0 * u_cnt / u_num * tex_u;
			vbuf[counter++].pos = vec3(curve_p.x, curve_p.y, curve_p.z);

			v_cnt++;
			v += increment;
			curve_p = vec4(0.0);	//��ʼ����������
		}
		u_cnt++;
		u += increment;
		v_cnt = 0;
		v = 0.0;		//��ʼ����������

		if (v - 1.0 > 1e-7) { u_cnt--; u = 1.0; };	//��u���Ҷ˵�
	}
	int iv[4];
	for (i = 0; i < u_num - 1; i++)
	{
		int base = i * u_num + patch_index * u_num * v_num;		// �� ����ֵ

		for (j = 0; j < v_num - 1; j++)
		{
			iv[0] = base + j;
			iv[1] = iv[0] + 1;
			iv[3] = iv[0] + u_num;
			iv[2] = iv[3] + 1;

			if (i % 2 == j % 2)
			{
				indices[iv_counter++] = iv[0];
				indices[iv_counter++] = iv[1];
				indices[iv_counter++] = iv[2];
				indices[iv_counter++] = iv[0];
				indices[iv_counter++] = iv[2];
				indices[iv_counter++] = iv[3];
			}
			else
			{
				indices[iv_counter++] = iv[0];
				indices[iv_counter++] = iv[1];
				indices[iv_counter++] = iv[3];
				indices[iv_counter++] = iv[1];
				indices[iv_counter++] = iv[2];
				indices[iv_counter++] = iv[3];
			}
		}
	}
	vec3 aver_normal;
	for (i = 0; i < iv_counter; i+=3)
	{
		aver_normal = TriangleNormal(
			vbuf[indices[i]].pos,
			vbuf[indices[i + 1]].pos,
			vbuf[indices[i + 2]].pos);
		vbuf[indices[i]].normal = aver_normal;
		vbuf[indices[i+1]].normal = aver_normal;
		vbuf[indices[i+1]].normal = aver_normal;
	}
	for (i = 0; i < iv_counter; i++)
	{
		vbuf[indices[i]].normal = normalize(vbuf[indices[i]].normal);
	}
}

void CMesh::CreateBezierObject(const char* filename, float increment, float tex_u, float tex_v)
{
	//���� Bezier ��������
	//filename : �����ļ���(�ļ���һ����  ���ƶ����� n �������� n ����  ������Ϣ��ÿһ���ɶ��Ÿ���,�ļ��� n+2 ��������Ƭ������ ������ÿ����ÿ������Ƭ������ֵ���ɶ��Ÿ���)
	//increment : ������������ [0, 1]ֱ�ӣ�����ϸ�ֵĳ̶ȣ�Խ�ӽ�0ϸ�ֲ��Խ��
	// tex_u:	(in) Texture coordinate multiplier in u direction
	// tex_v:   (in) Texture coordinate multiplier in v direction
	// u,v�ĺ����Ǳ����������u,v��

	if (increment <= 1e-6 || increment - 1.0 >= 1e-6) { cout << "����� increment �������Ϸ�,Ӧ���� (0, 1)��Χ�ڵĸ�����" << endl; exit(1); };

	int u_num = (int)(1 / increment) + 1, v_num = u_num;		//u,vϸ�ֺ�Ķ�������
	//printf("%s : u_num:%d,v_num:%d\n", filename, u_num, v_num);
	
	//��ȡ�ļ���Ϣ
	fstream iofile(filename, ios::in);			
	if (!iofile) { cout << "���ļ�ʧ�ܣ�" << filename << endl; exit(1); }//ʧ���˻ز���ϵͳ
	
	//������п��Ƶ����ݣ��ⲿ�ֽ��ļ���n+1�ж�ȡ���
	int control_point_num;	iofile >> control_point_num;//���Ƶ�����
	point3* cp_vertices = new point3[control_point_num];
	char sep; int i, j;	//�ָ���  \n   ѭ������i,j
	for (i = 0; i < control_point_num; i++)
	{
		iofile >> cp_vertices[i].x;
		iofile >> sep;
		iofile >> cp_vertices[i].y;
		iofile >> sep;
		iofile >> cp_vertices[i].z;
	}

	
	//ѭ���������Ƭ���������������� cp_iv[16] ָ��Ŀ��Ƶ���ϸ�ֵ�ǰ����Ƭ
	int patch_num;	iofile >> patch_num;//����Ƭ��(��n+2)������
	//������������ж����VAO����
	num_vertices = patch_num * u_num * v_num;	//patch_num������Ƭ, u, v��ÿ�����򶼻�ϸ�ֳ� (1 / increment)+1����
	CMeshVertex* vertices = new CMeshVertex[num_vertices];

	int cp_iv[16];		//����Ƭ��������
	int counter = 0;
	int iv_counter = 0;
	int cnt_patch;
	num_indices = patch_num * 6 * (u_num) * (v_num);//������
	GLuint* indices = new GLuint[num_indices];
	for (cnt_patch = 0; cnt_patch < patch_num; cnt_patch++)  //ѭ��ϸ��patch_num������Ƭ
	{
		for (j = 0; j < 16; j++)	//ÿ������Ƭ��16�����Ƶ����
		{
			iofile >> cp_iv[j];
			cp_iv[j] --;
			if (j < 15)
			{
				iofile >> sep;
			}
		}
		DivideBezierPatch(counter, iv_counter, cnt_patch, vertices, indices, cp_vertices, cp_iv, increment, tex_u, tex_v);
	}
	iofile.close();

	CreateGLResources(vertices, indices);

	delete[] vertices;

}
void CMesh::CreateAxes(float sx, float sy, float sz)
{
	primitive_type=GL_LINES;
	num_vertices=6;

	CMeshVertex vertices[6];
	const static color4 vcolors[3]={
		color4(1.0f, 0.0f, 0.0f, 1.0f),
		color4(0.0f, 1.0f, 0.0f, 1.0f),
		color4(0.0f, 0.0f, 1.0f, 1.0f)
	};

	vertices[0].pos=vec3(0.0f);
	vertices[0].color=vcolors[0];
	vertices[1].pos=vec3(sx, 0.0f, 0.0f);
	vertices[1].color=vcolors[0];

	vertices[2].pos=vec3(0.0f);
	vertices[2].color=vcolors[1];
	vertices[3].pos=vec3(0.0f, sy, 0.0f);
	vertices[3].color=vcolors[1];

	vertices[4].pos=vec3(0.0f);
	vertices[4].color=vcolors[2];
	vertices[5].pos=vec3(0.0f, 0.0f, sz);
	vertices[5].color=vcolors[2];

	CreateGLResources(vertices, NULL);
}
void CMesh::CreateCylinder(float radius, float h, int num_slices, int num_stacks, int num_rings, int tex_nx, int tex_ny)
{
	//printf("-----------------------------------------------------------------------------------------------\n");
	//printf("����Բ����");
	float dtheta = (M_PI + M_PI) / num_slices;
	float dh = h / num_stacks;

	int num_curve_vertices = (num_slices + 1) * (num_stacks + 1);		//׶�涥����
	int num_beside_curve_vertices = 2 * (num_slices + 1) * (num_rings + 1);	//�ϵ�����µ���
	num_vertices = num_curve_vertices + num_beside_curve_vertices;	//׶�涥���� + ���涥��������������Ȧ�Ķ����ǹ������㣩
	num_indices = 6 * num_slices * num_stacks + 2 * 6 * num_slices * num_rings;	//׶�涥��������+ ���涥��������

	CMeshVertex* vertices = new CMeshVertex[num_vertices];
	GLuint* indices = new GLuint[num_indices];

	//Բ����������ϸ��--�������
	int i, j, k;
	for (j = 0, k = 0; j <= num_stacks; j++)
	{
		float z = j * dh;
		float texcoord_t = (float)j / (float)num_stacks * tex_ny;
		for (i = 0; i <= num_slices; i++, k++)
		{
			float theta = dtheta * i;
			float ctheta = cos(theta);
			float stheta = sin(theta);
			vertices[k].pos.x = radius * ctheta;
			vertices[k].pos.y = radius * stheta;
			vertices[k].pos.z = z;
			vertices[k].color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
			vertices[k].texcoord.x = (float)i / (float)num_slices * tex_nx;
			vertices[k].texcoord.y = texcoord_t;
			vertices[k].normal = normalize(vec3(vertices[k].pos.x, vertices[k].pos.y, 0.0f) / radius * radius);
		}
	}
	//printf("-----------------------------------------------------------------------------------------------\n");
	//Բ����������--��������
	int iv[4];
	for (j = 0, k = 0; j < num_stacks; ++j)
	{
		int ibase = j * (num_slices + 1);
		for (i = 0; i < num_slices; ++i)
		{
			iv[0] = ibase + i;
			iv[1] = iv[0] + 1;
			iv[3] = iv[0] + num_slices + 1;
			iv[2] = iv[3] + 1;
			//printf("%d(iv[0]:%d,iv[1]:%d,iv[2]:%d,iv[3]:%d,)\n", k / 6, iv[0], iv[1], iv[2], iv[3]);
			if (i % 2 == j % 2)
			{
				indices[k++] = iv[0];
				indices[k++] = iv[1];
				indices[k++] = iv[2];
				indices[k++] = iv[0];
				indices[k++] = iv[2];
				indices[k++] = iv[3];
			}
			else
			{
				indices[k++] = iv[0];
				indices[k++] = iv[1];
				indices[k++] = iv[3];
				indices[k++] = iv[1];
				indices[k++] = iv[2];
				indices[k++] = iv[3];
			}
		}
	}
	//printf("-----------------------------------------------------------------------------------------------\n");
	//���涥��ϸ��--�������
	//(num_curve_vertices)-(num_curve_vertices + num_bottom_vertices/2)���µ��棬
	//(num_curve_vertices + num_bottom_vertices/2)-(num_curve_vertices + num_bottom_vertices)���ϵ��棬
	int topside_vertices_offset = num_beside_curve_vertices / 2;
	for (j = 0, k = num_curve_vertices; j <= num_rings; j++)
	{
		float dr = radius / num_rings;
		float texcoord_t = (float)j / (float)num_rings * tex_ny;
		for (i = 0; i <= num_slices; i++, k++)
		{
			float theta = dtheta * i;
			float ctheta = cos(theta);
			float stheta = sin(theta);

			vertices[k].pos.x = (radius - dr * j) * ctheta;			//�����µ���Ķ������
			vertices[k].pos.y = (radius - dr * j) * stheta;
			vertices[k].pos.z = 0;
			vertices[k].color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
			vertices[k].texcoord.x = (float)i / (float)num_slices * tex_nx;
			vertices[k].texcoord.y = texcoord_t;
			vertices[k].normal = (0.0f, 0.0f, -1.0f);

			vertices[k + topside_vertices_offset].pos.x = vertices[k].pos.x;		//�����ϵ���Ķ�����㣬ֱ�����µ���Ķ�����������ƽ�� h ����λ
			vertices[k + topside_vertices_offset].pos.y = vertices[k].pos.y;
			vertices[k + topside_vertices_offset].pos.z = h;
			vertices[k + topside_vertices_offset].color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
			vertices[k + topside_vertices_offset].texcoord.x = (float)i / (float)num_slices * tex_nx;
			vertices[k + topside_vertices_offset].texcoord.y = texcoord_t;
			vertices[k + topside_vertices_offset].normal = (0.0f, 0.0f, 1.0f);
		}
	}
	//printf("-----------------------------------------------------------------------------------------------\n");
	//�����������㣬�ϵ����������ֶ����µ����������ֶ�����Ϊ�ӵײ����������Ǳ���
	int topside_indices_offset = 6 * num_slices * num_rings;
	for (j = 0, k = 6 * num_slices * num_stacks; j < num_rings; ++j)
	{
		int ibase = j * (num_slices + 1);
		for (i = 0; i < num_slices; ++i)
		{
			iv[0] = ibase + i + num_curve_vertices;		//�����ibaseֵҪ�������涥��������ƫ����
			iv[1] = iv[0] + 1;
			iv[3] = iv[0] + num_slices + 1;
			iv[2] = iv[3] + 1;
			if (i % 2 == j % 2)
			{
				indices[k + topside_indices_offset] = iv[0] + topside_vertices_offset;		//�ϵ������������ƫ����������ĵ�һ�д�����ͬ
				indices[k++] = iv[0];	//�µ�������ֵ��������ڶ�����ͬ

				indices[k + topside_indices_offset] = iv[1] + topside_vertices_offset;
				indices[k++] = iv[2];

				indices[k + topside_indices_offset] = iv[2] + topside_vertices_offset;
				indices[k++] = iv[1];

				indices[k + topside_indices_offset] = iv[0] + topside_vertices_offset;
				indices[k++] = iv[0];

				indices[k + topside_indices_offset] = iv[2] + topside_vertices_offset;
				indices[k++] = iv[3];

				indices[k + topside_indices_offset] = iv[3] + topside_vertices_offset;
				indices[k++] = iv[2];
			}
			else
			{
				indices[k + topside_indices_offset] = iv[0] + topside_vertices_offset;		//�ϵ������������ƫ����������ĵ�һ�д�����ͬ
				indices[k++] = iv[0];	////�µ�������ֵ��������ڶ�����ͬ

				indices[k + topside_indices_offset] = iv[1] + topside_vertices_offset;
				indices[k++] = iv[3];

				indices[k + topside_indices_offset] = iv[3] + topside_vertices_offset;
				indices[k++] = iv[1];

				indices[k + topside_indices_offset] = iv[1] + topside_vertices_offset;
				indices[k++] = iv[1];

				indices[k + topside_indices_offset] = iv[2] + topside_vertices_offset;
				indices[k++] = iv[3];

				indices[k + topside_indices_offset] = iv[3] + topside_vertices_offset;
				indices[k++] = iv[2];
			}
		}
	}

	CreateGLResources(vertices, indices);

	delete[] vertices;
	delete[] indices;

}