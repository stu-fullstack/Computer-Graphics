#include <stddef.h>
#include "Mesh.h"

//计算线段的三等分点，length代表几等分点
point2 CalDividePoint(point2 a, point2 b, float length)
{
	point2 v;
	v.x = a.x + (b.x - a.x) * length;
	v.y = a.y + (b.y - a.y) * length;
	return v;
}

void CMesh::DivideLine(
	const point2& p0, const point2& p1, 
	CMeshVertex *vbuf, int& vcounter, int depth)
{
	//一条直线的分割，从左到右依次保存5个顶点,即对一条直线的分割会产生五个顶点。
	point2 oneStepPoints[5];
	int i;

	point2 divide_3_left = CalDividePoint(p0, p1, 1.0f / 3.0f);	//三等分点  左边的
	point2 divide_3_right = CalDividePoint(p0, p1, 2.0f / 3.0f);   //三等分点  右边的
	
	point2  divide_new_vertex;	//新三角形的顶点
	divide_new_vertex.x = (divide_3_right.x- divide_3_left.x) * cos(60.0f*DegreesToRadians) - (divide_3_right.y - divide_3_left.y) * sin(60.0f * DegreesToRadians);
	divide_new_vertex.y = (divide_3_right.x - divide_3_left.x) * sin(60.0f * DegreesToRadians) + (divide_3_right.y - divide_3_left.y) * cos(60.0f * DegreesToRadians);
	divide_new_vertex.x += divide_3_left.x;
	divide_new_vertex.y += divide_3_left.y;
	
	oneStepPoints[0] = p0;
	oneStepPoints[1] = divide_3_left;
	oneStepPoints[2] = divide_new_vertex;
	oneStepPoints[3] = divide_3_right;
	oneStepPoints[4] = p1;
	if (depth>0)
	{
		for (i = 0;i < 4; i++)
		{
			DivideLine(oneStepPoints[i], oneStepPoints[i + 1], vbuf, vcounter, depth-1);
		}
	}
	else
	{
		for (i = 0; i < 4; i++)
		{
			vbuf[vcounter].color = color4(1.0f, 0.0f, 0.0f, 1.0f);
			vbuf[vcounter++].pos = oneStepPoints[i];
			vbuf[vcounter].color = color4(0.0f, 0.0f, 0.0f, 1.0f);
			vbuf[vcounter++].pos = oneStepPoints[i+1];
		}
	}
}

CMesh::CMesh(void)
{
	prmitive_type=GL_LINES;
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

void CMesh::CreateKochSnowflate(
	const point2 snow_vertices[3],
	int subdivision_depth)
{
	//计算顶点个数   3*pow(4, subdivision_depth+1)   是边数，画图是用直线来画的，顶点数量要乘以2
	num_vertices = (3*pow(4, subdivision_depth+1))*2;
	CMeshVertex *vertices=new CMeshVertex [num_vertices];
	int i = 0;
	DivideLine(snow_vertices[0], snow_vertices[1], vertices, i, subdivision_depth);
	DivideLine(snow_vertices[1], snow_vertices[2], vertices, i, subdivision_depth);
	DivideLine(snow_vertices[2], snow_vertices[0], vertices, i, subdivision_depth);
	/*for (int i = 0; i < num_vertices; i++)   //看一下顶点的计算情况
	{
		printf("-------v%d(%.1f,%.1f)\n", i, (vertices+i)->pos.x, (vertices+i)->pos.y);
	}*/
	glGenVertexArrays(1, &vertex_array_obj);
	glBindVertexArray(vertex_array_obj);

	glGenBuffers(1, &vertex_buffer_obj);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_obj);
	glBufferData(GL_ARRAY_BUFFER, 
		sizeof(CMeshVertex)* i,
		vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 
		sizeof(CMeshVertex), (GLvoid *)offsetof(CMeshVertex, pos));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 
		sizeof(CMeshVertex), (GLvoid *)offsetof(CMeshVertex, color));

	glBindVertexArray(0);

	delete [] vertices;
}
