#include "GL/glew.h"
#include "GL/freeglut.h"
#include "vec.h"
#include <stdlib.h>
#include "GLHelper.h"
#include "Mesh.h"
#include "math.h"

GLuint g_GLSL_prog;
CMesh g_obj;

void init_shaders(void)
{
	g_GLSL_prog=InitShader(
		"..\\shaders\\simple_with_color-vs.txt",
		"..\\shaders\\simple_with_color-fs.txt");
}

void init_scene(void)
{
	// 新三角形顶点在外侧（凹侧）的初始数据
	point2 snow_vertices[3]={
		point2(-0.433f,-0.25f),
		point2(0.0f, 0.5f),
		point2(0.433f, -0.25f)
	};

	// 新三角形顶点在内侧（凹侧）的初始数据
	//point2 snow_vertices[3] = {
	//	point2(-0.433f,0.25f),
	//	point2(0.0f, -0.5f),
	//	point2(0.433f, 0.25f)
	//};

	//经测试，我的运行环境最多只能迭代9次
	int subdivision_depth = 4;
	if(subdivision_depth > 0)
		g_obj.CreateKochSnowflate(snow_vertices,subdivision_depth-1);
}

void init(void)
{
	init_shaders();
	init_scene();
	glLineWidth(4.0);
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
	glViewport(0, 0, w, h);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutCreateWindow("Koch Snowflate");

	glewExperimental=GL_TRUE;
	glewInit();

	init();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	glutMainLoop();



	return 0;
}