#include <stddef.h>
#include "Mesh.h"
#include <stdlib.h>

void CMesh::DivideTriangle(
	const point2& v0, const point2& v1, const point2& v2, 
	CMeshVertex *vbuf, int& vcounter, int depth)
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
		for (int i=0; i<4; ++i)
		{
			vbuf[vcounter].color=base_colors[i];
			vbuf[vcounter++].pos=*pv[iv_face[i][0]];
			vbuf[vcounter].color=base_colors[i];
			vbuf[vcounter++].pos=*pv[iv_face[i][1]];
			vbuf[vcounter].color=base_colors[i];
			vbuf[vcounter++].pos=*pv[iv_face[i][2]];
		}
	}
}

CMesh::CMesh(void)
{
	prmitive_type=GL_TRIANGLES;
	num_vertices=0;
	num_indices=0;
	vertex_array_obj=0;
	vertex_buffer_obj=0;
	index_buffer_obj=0;
}

void CMesh::ReleaseGLResources(void)
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
{
	glBindVertexArray(vertex_array_obj);
	if (index_buffer_obj==0)
		glDrawArrays(prmitive_type, 0, num_vertices);
	else
		glDrawElements(prmitive_type, num_indices, 
			GL_UNSIGNED_INT, (GLvoid *)0);
	glBindVertexArray(0);
}

void CMesh::CreateGLResources(
	CMeshVertex *vertices, GLuint *indices)
{
	glGenVertexArrays(1, &vertex_array_obj);
	glBindVertexArray(vertex_array_obj);

	glGenBuffers(1, &vertex_buffer_obj);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_obj);
	glBufferData(GL_ARRAY_BUFFER, 
		sizeof(CMeshVertex)*num_vertices,
		vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 
		sizeof(CMeshVertex), (GLvoid *)offsetof(CMeshVertex, pos));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 
		sizeof(CMeshVertex), (GLvoid *)offsetof(CMeshVertex, color));

	if (indices!=NULL)
	{
		glGenBuffers(1, &index_buffer_obj);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_obj);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
			sizeof(GLuint)*num_indices,
			indices, GL_STATIC_DRAW);
	}

	glBindVertexArray(0);
}

void CMesh::CreateGasket2D(
	const point2 triangle_vertices[3],
	int subdivision_depth)
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

void CMesh::CreateCube(float size)
{
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

	static const vec3 nface[6]={
		vec3(1.0f, 0.0f, 0.0f),
		vec3(-1.0f, 0.0f, 0.0f),
		vec3(0.0f, 1.0f, 0.0f),
		vec3(0.0f, -1.0f, 0.0f),
		vec3(0.0f, 0.0f, 1.0f),
		vec3(0.0f, 0.0f, -1.0f)
	};

	static const int iv_face[6][4]={
		{1, 2, 6, 5},
		{3, 0, 4, 7},
		{2, 3, 7, 6},
		{0, 1, 5, 4},
		{4, 5, 6, 7},
		{1, 0, 3, 2}
	};

	num_vertices=24;
	num_indices=36;

	CMeshVertex *vertices=new CMeshVertex [num_vertices];
	GLuint *indices=new GLuint [num_indices];

	int i, k;
	float sh=0.5f*size;
	for (i=0; i<6; ++i)
	{
		int iv0=i*4;
		
		int counter=iv0;
		for (k=0; k<4; ++k, ++counter)
		{
			vertices[counter].pos=vpos[iv_face[i][k]]*sh;
			vertices[counter].color=color4(0.0f, 0.0f, 0.0f, 1.0f);
		}

		k=i*6;
		indices[k++]=iv0;
		indices[k++]=iv0+1;
		indices[k++]=iv0+2;
		indices[k++]=iv0;
		indices[k++]=iv0+2;
		indices[k++]=iv0+3;
	}

	CreateGLResources(vertices, indices);

	delete [] vertices;
	delete [] indices;
}

void CMesh::CreateRect(float sx, float sy, int nx, int ny)
{
	float dx=sx/nx;
	float dy=sy/ny;
	float sx_h=0.5f*sx;
	float sy_h=0.5f*sy;

	num_vertices=(nx+1)*(ny+1);
	num_indices=6*nx*ny;

	CMeshVertex *vertices=new CMeshVertex [num_vertices];
	GLuint *indices=new GLuint [num_indices];

	int i, j, k;
	for (j=0, k=0; j<=ny; ++j)
	{
		float y=-sy_h+dy*j;
		for (i=0; i<=nx; ++i, ++k)
		{
			float x=-sx_h+dx*i;
			vertices[k].pos.x=x;
			vertices[k].pos.y=y;
			vertices[k].pos.z=0.0f;
			vertices[k].color=vec4(0.0f, 0.0f, 0.0f, 0.0f);
		}
	}

	int iv[4];
	for (j=0, k=0; j<ny; ++j)
	{
		int ibase=j*(nx+1);
		for (i=0; i<nx; ++i)
		{
			iv[0]=ibase+i;
			iv[1]=iv[0]+1;
			iv[3]=iv[0]+nx+1;
			iv[2]=iv[3]+1;

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

	CreateGLResources(vertices, indices);

	delete [] vertices;
	delete [] indices;
}

void CMesh::CreateSphere(
	float radius, int num_slices, int num_stacks)
{	
	float dtheta=(M_PI+M_PI)/num_slices;
	float dphi=M_PI/num_stacks;

	num_vertices=(num_slices+1)*(num_stacks+1);
	num_indices=6*num_slices*num_stacks;

	CMeshVertex *vertices=new CMeshVertex [num_vertices];
	GLuint *indices=new GLuint [num_indices];

	int i, j, k;
	for (j=0, k=0; j<=num_stacks; ++j)
	{
		float phi=-0.5*M_PI+dphi*j;
		float cphi=cos(phi);
		float sphi=sin(phi);
		for (i=0; i<=num_slices; ++i, ++k)
		{
			float theta=dtheta*i;
			float ctheta=cos(theta);
			float stheta=sin(theta);
			vertices[k].pos.x=radius*cphi*ctheta;
			vertices[k].pos.y=radius*cphi*stheta;
			vertices[k].pos.z=radius*sphi;
			vertices[k].color=vec4(0.0f, 0.0f, 0.0f, 0.0f);
			//printf("%d(%f,%f,%f)(dtheta:%f,dphi:%f,phi:%f,theta:%f)\n", k,vertices[k].pos.x, vertices[k].pos.y, vertices[k].pos.z,dtheta,dphi,phi,theta);
		}
	}

	int iv[4];
	for (j=0, k=0; j<num_stacks; ++j)
	{
		int ibase=j*(num_slices+1);
		for (i=0; i<num_slices; ++i)
		{
			iv[0]=ibase+i;
			iv[1]=iv[0]+1;
			iv[3]=iv[0]+num_slices+1;
			iv[2]=iv[3]+1;

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
	CreateGLResources(vertices, indices);

	delete [] vertices;
	delete [] indices;
}

void CMesh::CreateCone(float radius, float h, int num_slices, int num_stacks, int num_rings)
{
	//printf("-----------------------------------------------------------------------------------------------\n");
	//printf("创建圆锥");
	float dtheta = (M_PI + M_PI) / num_slices;
	float dh = h / num_stacks;

	int num_curve_vertices = (num_slices + 1) * (num_stacks + 1);	//锥面顶点数
	int num_bottom_vertices = (num_slices + 1) * (num_rings + 1);	//底面顶点数(为做统一处理，底面圆上的点将重复使用2次，额外开销小并且方便计算)
	num_vertices = num_curve_vertices + num_bottom_vertices;	//锥面顶点数 + 底面顶点数
	num_indices = 6 * num_slices * num_stacks + 6 * num_slices * num_rings;	//锥面顶点索引数+ 底面顶点索引数
	
	CMeshVertex* vertices = new CMeshVertex[num_vertices];
	GLuint* indices = new GLuint[num_indices];	

	//锥面顶点细分--顶点计算
	int i, j, k;
	for (j = 0, k = 0; j <= num_stacks; j++)
	{
		float z = j * dh;
		for (i = 0; i <= num_slices; i++,k++)
		{
			float theta = dtheta * i;
			float ctheta = cos(theta);
			float stheta = sin(theta);
			vertices[k].pos.x = radius * (1 - z/h) * ctheta;
			vertices[k].pos.y = radius * (1 - z/h) * stheta;
			vertices[k].pos.z = z;
			vertices[k].color = vec4(1.0f * ((rand() % 90) + 10) / 100.0f,
									 1.0f * ((rand() % 90) + 10) / 100.0f,
									 1.0f * ((rand() % 90) + 10) / 100.0f,
								 	 0.0f);
			//printf("%d(%f,%f,%f)\n", k,vertices[k].pos.x, vertices[k].pos.y, vertices[k].pos.z);
		}
	}
	//printf("-----------------------------------------------------------------------------------------------\n");
	//锥面索引计算
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
	//底面顶点细分--顶点计算
	for (j = 0, k = num_curve_vertices; j <= num_rings; j++)
	{
		float dr = radius/num_rings;	//底面半径微元
		for (i = 0; i <= num_slices; i++, k++)
		{
			float theta = dtheta * i;
			float ctheta = cos(theta);
			float stheta = sin(theta);
			vertices[k].pos.x = (radius - dr * j) * ctheta;
			vertices[k].pos.y = (radius - dr * j) * stheta;
			vertices[k].pos.z = 0;
			vertices[k].color = vec4(0.6f * ((rand() % 100) + 10) / 100.0f,
									0.8f  * ((rand() % 100) + 10) / 100.0f,
									1.0f  * ((rand() % 100) + 10) / 100.0f,
									0.0f);
			//printf("%d(%f,%f,%f)\n", k,vertices[k].pos.x, vertices[k].pos.y, vertices[k].pos.z);
		}
	}
	printf("-----------------------------------------------------------------------------------------------\n");
	//底面索引计算，注意底面遵守左手定则，因为从底部看看到的是背面
	for (j = 0, k = 6 * num_slices * num_stacks; j < num_rings; ++j)
	{
		int ibase = j * (num_slices + 1);
		for (i = 0; i < num_slices; ++i)
		{
			iv[0] = ibase + i + num_curve_vertices;
			iv[1] = iv[0] + 1 ;
			iv[3] = iv[0] + num_slices + 1 ;
			iv[2] = iv[3] + 1 ;
			//printf("%d(iv[0]:%d,iv[1]:%d,iv[2]:%d,iv[3]:%d,)\n", k / 6, iv[0] - num_curve_vertices, iv[1]- num_curve_vertices, iv[2]- num_curve_vertices, iv[3]- num_curve_vertices);
			if (i % 2 == j % 2)
			{
				indices[k++] = iv[0];
				indices[k++] = iv[2];
				indices[k++] = iv[1];
				indices[k++] = iv[0];
				indices[k++] = iv[3];
				indices[k++] = iv[2];
			}
			else
			{
				indices[k++] = iv[0];
				indices[k++] = iv[3];
				indices[k++] = iv[1];
				indices[k++] = iv[1];
				indices[k++] = iv[3];
				indices[k++] = iv[2];
			}
		}
	}
	//printf("**********(k:%d,num_vertices:%d,num_indices:%d)***************\n",k, num_vertices, num_indices);

	CreateGLResources(vertices, indices);

	delete[] vertices;
	delete[] indices;
}
void CMesh::CreateCylinder(float radius, float h, int num_slices, int num_stacks, int num_rings)
{
	//printf("-----------------------------------------------------------------------------------------------\n");
	//printf("创建圆柱体");
	float dtheta = (M_PI + M_PI) / num_slices;
	float dh = h / num_stacks;

	int num_curve_vertices = (num_slices + 1) * (num_stacks + 1);		//锥面顶点数
	int num_beside_curve_vertices = 2*(num_slices + 1) * (num_rings + 1);	//上底面和下底面
	num_vertices = num_curve_vertices + num_beside_curve_vertices;	//锥面顶点数 + 底面顶点数（底面最外圈的顶点是公共顶点）
	num_indices = 6 * num_slices * num_stacks +2 * 6 * num_slices * num_rings;	//锥面顶点索引数+ 底面顶点索引数

	CMeshVertex* vertices = new CMeshVertex[num_vertices];
	GLuint* indices = new GLuint[num_indices];

	//圆柱体侧边曲面细分--顶点计算
	int i, j, k;
	for (j = 0, k = 0; j <= num_stacks; j++)
	{
		float z = j * dh;
		for (i = 0; i <= num_slices; i++, k++)
		{
			float theta = dtheta * i;
			float ctheta = cos(theta);
			float stheta = sin(theta);
			vertices[k].pos.x = radius * ctheta;
			vertices[k].pos.y = radius * stheta;
			vertices[k].pos.z = z;
			vertices[k].color = vec4(1.0f * ((rand() % 90) + 10) / 100.0f,
									1.0f * ((rand() % 90) + 10) / 100.0f,
									1.0f * ((rand() % 90) + 10) / 100.0f,
									0.0f);
			//printf("%d(%f,%f,%f)\n", k, vertices[k].pos.x, vertices[k].pos.y, vertices[k].pos.z);

		}
	}
	//printf("-----------------------------------------------------------------------------------------------\n");
	//圆柱体侧边曲面--索引计算
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
	//底面顶点细分--顶点计算
	//(num_curve_vertices)-(num_curve_vertices + num_bottom_vertices/2)是下底面，
	//(num_curve_vertices + num_bottom_vertices/2)-(num_curve_vertices + num_bottom_vertices)是上底面，
	int topside_vertices_offset = num_beside_curve_vertices / 2;
	for (j = 0, k = num_curve_vertices; j <= num_rings; j++)
	{
		float dr = radius / num_rings;
		for (i = 0; i <= num_slices; i++, k++)
		{
			float theta = dtheta * i;
			float ctheta = cos(theta);
			float stheta = sin(theta);
			
			vertices[k].pos.x = (radius - dr * j) * ctheta;			//这是下底面的顶点计算
			vertices[k].pos.y = (radius - dr * j) * stheta;
			vertices[k].pos.z = 0;
			vertices[k].color = vec4(0.5f * ((rand() % 90) + 10) / 100.0f,
									 0.5f * ((rand() % 90) + 10) / 100.0f,
									 0.5f * ((rand() % 90) + 10) / 100.0f,
									 0.0f);
			
			vertices[k + topside_vertices_offset].pos.x = vertices[k].pos.x;		//这是上底面的顶点计算，直接是下底面的顶点整体往上平移 h 个单位
			vertices[k + topside_vertices_offset].pos.y = vertices[k].pos.y;
			vertices[k + topside_vertices_offset].pos.z = h;
			vertices[k + topside_vertices_offset].color = vec4(0.8f * ((rand() % 90) + 10) / 100.0f,
															   0.8f * ((rand() % 90) + 10) / 100.0f,
														       0.8f * ((rand() % 90) + 10) / 100.0f,
															   0.0f);
			//printf("%d(%f,%f,%f)%f\n", k + topside_vertices_offset, vertices[k].pos.x, vertices[k].pos.y, vertices[k].pos.z,dr);
			//printf("off%d(%f,%f,%f)%f\n", k + topside_vertices_offset, vertices[k + topside_vertices_offset].pos.x, vertices[k + topside_vertices_offset].pos.y, vertices[k + topside_vertices_offset].pos.z,dr);

		}
	}
	//printf("-----------------------------------------------------------------------------------------------\n");
	//底面索引计算，上底面遵守右手定则，下底面遵守左手定则，因为从底部看看到的是背面
	int topside_indices_offset = 6 * num_slices * num_rings;
	for (j = 0, k = 6 * num_slices * num_stacks; j < num_rings; ++j)
	{
		int ibase = j * (num_slices + 1);
		for (i = 0; i < num_slices; ++i)
		{
			iv[0] = ibase + i + num_curve_vertices;		//底面的ibase值要加上曲面顶点数量的偏移量
			iv[1] = iv[0] + 1;
			iv[3] = iv[0] + num_slices + 1;
			iv[2] = iv[3] + 1;
			//printf("%d(iv[0]:%d,iv[1]:%d,iv[2]:%d,iv[3]:%d,)\n", k / 6, iv[0] - num_curve_vertices, iv[1] - num_curve_vertices, iv[2] - num_curve_vertices, iv[3] - num_curve_vertices);
			if (i % 2 == j % 2)
			{
				indices[k + topside_indices_offset] = iv[0] + topside_vertices_offset;		//上底面的索引加上偏移量，下面的第一行处理都相同
				indices[k++] = iv[0];	//下底面索引值，和下面第二行相同

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
				indices[k + topside_indices_offset] = iv[0] + topside_vertices_offset;		//上底面的索引加上偏移量，下面的第一行处理都相同
				indices[k++] = iv[0];	////下底面索引值，和下面第二行相同

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
	//printf("**********(k:%d,num_vertices:%d,num_indices:%d)***************\n", k, num_vertices, num_indices);

	CreateGLResources(vertices, indices);

	delete[] vertices;
	delete[] indices;

}