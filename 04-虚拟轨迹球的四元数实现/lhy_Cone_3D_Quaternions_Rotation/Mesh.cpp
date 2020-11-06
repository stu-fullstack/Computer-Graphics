#include <stddef.h>
#include <stdlib.h>
#include "Mesh.h"

CMesh::CMesh(void)
{
	prmitive_type = GL_TRIANGLES;
	num_vertices = 0;
	num_indices = 0;
	vertex_array_obj = 0;
	vertex_buffer_obj = 0;
	index_buffer_obj = 0;
}

void CMesh::ReleaseGLResources(void)
{
	if (vertex_array_obj != 0)
		glDeleteVertexArrays(1, &vertex_array_obj);
	vertex_array_obj = 0;

	if (vertex_buffer_obj != 0)
		glDeleteBuffers(1, &vertex_buffer_obj);
	vertex_buffer_obj = 0;

	if (index_buffer_obj != 0)
		glDeleteBuffers(1, &index_buffer_obj);
	index_buffer_obj = 0;
}

void CMesh::Draw(void)
{
	glBindVertexArray(vertex_array_obj);
	if (index_buffer_obj == 0)
		glDrawArrays(prmitive_type, 0, num_vertices);
	else
		glDrawElements(prmitive_type, num_indices,
			GL_UNSIGNED_INT, (GLvoid*)0);
	glBindVertexArray(0);
}

void CMesh::CreateGLResources(
	CMeshVertex* vertices, GLuint* indices)
{
	glGenVertexArrays(1, &vertex_array_obj);
	glBindVertexArray(vertex_array_obj);

	glGenBuffers(1, &vertex_buffer_obj);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_obj);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(CMeshVertex) * num_vertices,
		vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
		sizeof(CMeshVertex), (GLvoid*)offsetof(CMeshVertex, pos));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE,
		sizeof(CMeshVertex), (GLvoid*)offsetof(CMeshVertex, color));

	if (indices != NULL)
	{
		glGenBuffers(1, &index_buffer_obj);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_obj);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			sizeof(GLuint) * num_indices,
			indices, GL_STATIC_DRAW);
	}

	glBindVertexArray(0);
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
		for (i = 0; i <= num_slices; i++, k++)
		{
			float theta = dtheta * i;
			float ctheta = cos(theta);
			float stheta = sin(theta);
			vertices[k].pos.x = radius * (1 - z / h) * ctheta;
			vertices[k].pos.y = radius * (1 - z / h) * stheta;
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
		float dr = radius / num_rings;	//底面半径微元
		for (i = 0; i <= num_slices; i++, k++)
		{
			float theta = dtheta * i;
			float ctheta = cos(theta);
			float stheta = sin(theta);
			vertices[k].pos.x = (radius - dr * j) * ctheta;
			vertices[k].pos.y = (radius - dr * j) * stheta;
			vertices[k].pos.z = 0;
			vertices[k].color = vec4(0.6f * ((rand() % 100) + 10) / 100.0f,
				0.8f * ((rand() % 100) + 10) / 100.0f,
				1.0f * ((rand() % 100) + 10) / 100.0f,
				0.0f);
			//printf("%d(%f,%f,%f)\n", k,vertices[k].pos.x, vertices[k].pos.y, vertices[k].pos.z);
		}
	}
	//printf("-----------------------------------------------------------------------------------------------\n");
	//底面索引计算，注意底面遵守左手定则，因为从底部看看到的是背面
	for (j = 0, k = 6 * num_slices * num_stacks; j < num_rings; ++j)
	{
		int ibase = j * (num_slices + 1);
		for (i = 0; i < num_slices; ++i)
		{
			iv[0] = ibase + i + num_curve_vertices;
			iv[1] = iv[0] + 1;
			iv[3] = iv[0] + num_slices + 1;
			iv[2] = iv[3] + 1;
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