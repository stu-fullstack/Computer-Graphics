#include <stddef.h>
#include "Mesh.h"

CMesh::CMesh(void)
{
	prmitive_type=GL_TRIANGLES;
	num_vertices=0;
	vertex_array_obj=0;
	vertex_buffer_obj=0;
}

void CMesh::ReleaseGLResources(void)
{
	if (vertex_array_obj!=0)
		glDeleteVertexArrays(1, &vertex_array_obj);
	vertex_array_obj=0;

	if (vertex_buffer_obj!=0)
		glDeleteBuffers(1, &vertex_buffer_obj);
	vertex_buffer_obj=0;
}

void CMesh::Draw(void)
{
	glBindVertexArray(vertex_array_obj);
	glDrawArrays(prmitive_type, 0, num_vertices);
	glBindVertexArray(0);
}

void CMesh::CreateGLResources(CMeshVertex *vertices)
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

	glBindVertexArray(0);
}

void CMesh::CreateGenericModel(int num_vertices_in)
{
	num_vertices=num_vertices_in;
	CreateGLResources(NULL);
}
