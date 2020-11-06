#include "GL/glew.h"
#include "GL/freeglut.h"
#include "vec.h"
#include <stdlib.h>
#include <stddef.h>
#include "GLHelper.h"
#include "Mesh.h"

#define N 583		//支持的最大顶点数，一共能画N/3个三角形

int vcounter = 0;	//已经画的顶点个数,这个变量的位置关系会在画图的时候使用
int g_window_width, g_window_height;

GLuint g_GLSL_prog;
CMesh g_obj;

point3 g_line_seg_vertices[N];
int g_interactive_drawing_mode=0;

void MousePosToNormalizedPos(
	float mouse_pos_x, float mouse_pos_y,
	float& normalized_pos_x, float& normalized_pos_y)
{
	normalized_pos_x=2.0f*mouse_pos_x/g_window_width-1.0f;
	normalized_pos_y=2.0f*(g_window_height-mouse_pos_y)/g_window_height-1.0f;
}

void CopyVertexPositionToGPU(int vertex_id)
{
	glBindBuffer(GL_ARRAY_BUFFER, g_obj.vertex_buffer_obj);
	glBufferSubData(GL_ARRAY_BUFFER,  
		sizeof(CMeshVertex)*vertex_id+offsetof(CMeshVertex, pos),
		sizeof(point3), g_line_seg_vertices+vertex_id);
}

void init_shaders(void)
{
	g_GLSL_prog=InitShader(
		"..\\shaders\\simple_with_color-vs.txt",
		"..\\shaders\\simple_with_color-fs.txt");
}

void init_scene(void)
{
	g_obj.CreateGenericModel(N);
	g_obj.prmitive_type=GL_LINES;
	
	CMeshVertex v[N];
	int i;
	for (i = 0; i < N; i++)
	{
		v[i].pos = g_line_seg_vertices[i];
		v[i].color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}
	glBindBuffer(GL_ARRAY_BUFFER, g_obj.vertex_buffer_obj);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 
		sizeof(CMeshVertex)*N, v);
}

void init(void)
{
	init_shaders();
	init_scene();
	glClearColor(1.0, 1.0, 1.0, 0.0);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(g_GLSL_prog);

	g_obj.Draw();

	glFlush();
	glutSwapBuffers();
}

void reshape(int w, int h)
{
	g_window_width=w;
	g_window_height=h;
	glViewport(0, 0, w, h);
}

void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		g_interactive_drawing_mode = 1;
		int i = vcounter % 6;	//用i来映射目“前在画第几条边”的信息
		if (state == GLUT_DOWN && i == 0)		//i=0表示从初始点出发画第一条边
		{
			MousePosToNormalizedPos(x, y,
				g_line_seg_vertices[vcounter].x,
				g_line_seg_vertices[vcounter].y);
			g_line_seg_vertices[vcounter + 1] = g_line_seg_vertices[vcounter];	//做个微偏移处理
			CopyVertexPositionToGPU(vcounter);
			CopyVertexPositionToGPU(vcounter + 1);
			glutPostRedisplay();
		}
		else if (state == GLUT_DOWN && i == 2)	//i=2表示画第二条边
		{
			g_line_seg_vertices[vcounter] = g_line_seg_vertices[vcounter - 1];		//第二条边的出发点是前一条边的终点
			MousePosToNormalizedPos(x, y,
				g_line_seg_vertices[vcounter + 1].x,
				g_line_seg_vertices[vcounter + 1].y);
			CopyVertexPositionToGPU(vcounter);
			CopyVertexPositionToGPU(vcounter + 1);
			glutPostRedisplay();
		}
		else if (state == GLUT_DOWN && i == 4)	//i=4表示画第三条边
		{
			g_line_seg_vertices[vcounter] = g_line_seg_vertices[vcounter - 1];		//第三条边的出发点是前一条边的终点
			g_line_seg_vertices[vcounter + 1] = g_line_seg_vertices[vcounter - 4];		//第三条边的终点是出发点
			CopyVertexPositionToGPU(vcounter);
			CopyVertexPositionToGPU(vcounter + 1);
			glutPostRedisplay();
		}
		else if (state == GLUT_UP)
		{
			vcounter += 2;	//表示鼠标左键松开了，所以画完了一条边，顶点的序号加2（一条边要两个顶点存储）
			g_interactive_drawing_mode = 0;
		}
	}
}

void mouse_motion(int x, int y)
{
	int i = vcounter % 6;	//用i来映射目“前在画第几条边”的信息，同上
	if (g_interactive_drawing_mode && i < 4)	//鼠标移动时第一条和第二条边处理方式一样
	{
		MousePosToNormalizedPos(x, y,
			g_line_seg_vertices[vcounter + 1].x,
			g_line_seg_vertices[vcounter + 1].y);
		CopyVertexPositionToGPU(vcounter + 1);
		glutPostRedisplay();
	}
	else	//第三条边
	{
		CopyVertexPositionToGPU(vcounter);
	}
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutCreateWindow("Interactive line drawing");

	glewExperimental=GL_TRUE;
	glewInit();

	init();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(mouse_motion);

	glutMainLoop();

	return 0;
}
