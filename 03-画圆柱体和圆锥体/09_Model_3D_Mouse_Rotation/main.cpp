#include "GL/glew.h"
#include "GL/freeglut.h"
#include "vec.h"
#include <stdlib.h>
#include "mat.h"
#include "GLHelper.h"
#include "Mesh.h"

#define MENU_ITEM_MODEL_RECT   0
#define MENU_ITEM_MODEL_CUBE   1
#define MENU_ITEM_MODEL_SPHERE 2
#define MENU_ITEM_MODEL_GASKET 3

//定义圆柱体和圆锥
#define MENU_ITEM_MODEL_CYLINDER 4
#define MENU_ITEM_MODEL_CONE 5
#define MENU_ITEM_POLYGON_MODE_LINE 10
#define MENU_ITEM_POLYGON_MODE_FILL 11

CMesh g_obj[6];
int g_current_obj_id=MENU_ITEM_MODEL_CONE;

GLuint g_GLSL_prog;
int g_model_matrix_loc;
int g_mouse_rotation_mode=0;
int g_mouse_x, g_mouse_y;
vec3 g_mouse_vec_start;
mat4 g_model_rotation, g_model_rotation_prev;

mat4 CalculateRotationMatrixInc(
	float x_rotation_angle, float y_rotation_angle, float angle_delta)
{
	int nx=(int)(fabs(x_rotation_angle)/angle_delta);
	int ny=(int)(fabs(y_rotation_angle)/angle_delta);
	int n=(nx>ny)?nx:ny;
	if (n==0) n=1;
	float dx=x_rotation_angle/n;
	float dy=y_rotation_angle/n;
	mat4 dM=RotateX(dx)*RotateY(dy);
	mat4 M=dM;
	for (int i=1; i<n; i++)
		M*=dM;
	return M;
}

void init_shaders(void)
{
	g_GLSL_prog=InitShader(
		"..\\shaders\\simple3D-vs.txt",
		"..\\shaders\\simple3D-fs.txt");

	mat4 M;
	int loc=glGetUniformLocation(g_GLSL_prog, "view_matrix");
	glUniformMatrix4fv(loc, 1, GL_TRUE, M);
	g_model_matrix_loc=glGetUniformLocation(g_GLSL_prog, "model_matrix");
}

void init_scene(void)
{
	point3 tetra_vertices[4]={
		point3(-0.8f, -0.6f, 0.6f),
		point3(0.8f, -0.6f, 0.6f),
		point3(0.0f, 0.8f, 0.6f),
		point3(0.0f, 0.0f, -0.8f),
	};

	g_obj[MENU_ITEM_MODEL_RECT].CreateRect(1.5f, 1.0f, 30, 20);
	g_obj[MENU_ITEM_MODEL_CUBE].CreateCube(1.0f);
	g_obj[MENU_ITEM_MODEL_SPHERE].CreateSphere(0.5f, 32, 32);
	g_obj[MENU_ITEM_MODEL_GASKET].CreateGasket3D(tetra_vertices, 4);
	g_obj[MENU_ITEM_MODEL_CYLINDER].CreateCylinder(0.5f, 0.8f, 32, 32, 8);
	g_obj[MENU_ITEM_MODEL_CONE].CreateCone(0.8f, 1.0f, 32, 32, 8);
}

void main_menu_func(int menu_id)
{
}

void model_selection_menu_func(int menu_id)
{
	g_current_obj_id=menu_id;
	glutPostRedisplay();
}

void polygon_mode_selection_menu_func(int menu_id)
{
	switch(menu_id)
	{
		case MENU_ITEM_POLYGON_MODE_LINE:
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			break;
		case MENU_ITEM_POLYGON_MODE_FILL:
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			break;
	}
	glutPostRedisplay();
}

void init(void)
{
	init_shaders();
	init_scene();

	glClearColor(1.0, 1.0, 1.0, 0.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	int model_selection_menu_id=glutCreateMenu(model_selection_menu_func);
	glutAddMenuEntry("Rectangle", MENU_ITEM_MODEL_RECT);
	glutAddMenuEntry("Cube", MENU_ITEM_MODEL_CUBE);
	glutAddMenuEntry("Sphere", MENU_ITEM_MODEL_SPHERE);
	glutAddMenuEntry("Gasket", MENU_ITEM_MODEL_GASKET);
	glutAddMenuEntry("Cylinder", MENU_ITEM_MODEL_CYLINDER);
	glutAddMenuEntry("Cone", MENU_ITEM_MODEL_CONE);

	int polygon_mode_selection_menu_id=glutCreateMenu(polygon_mode_selection_menu_func);
	glutAddMenuEntry("Line", MENU_ITEM_POLYGON_MODE_LINE);
	glutAddMenuEntry("Fill", MENU_ITEM_POLYGON_MODE_FILL);

	glutCreateMenu(main_menu_func);
	glutAddSubMenu("Select Model", model_selection_menu_id);
	glutAddSubMenu("Select Polygon Mode", polygon_mode_selection_menu_id);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(g_GLSL_prog);

	mat4 M;
	M=Translate(0.0f, 0.0f, -2.0f)*g_model_rotation;
	glUniformMatrix4fv(g_model_matrix_loc, 1, GL_TRUE, M);

	g_obj[g_current_obj_id].Draw();

	glFlush();
	glutSwapBuffers();
}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);

	glUseProgram(g_GLSL_prog);
	int loc=glGetUniformLocation(g_GLSL_prog, "projection_matrix");
	mat4 M;
	M=Perspective(60.0f, (float)w/(float)h, 0.1f, 4.0f);
	glUniformMatrix4fv(loc, 1, GL_TRUE, M);
}

void mouse(int button, int state, int x, int y)
{
	if (button==GLUT_LEFT_BUTTON)
	{
		if (state==GLUT_DOWN)
		{
			g_mouse_rotation_mode=1;
			g_mouse_x=x;
			g_mouse_y=y;
			g_model_rotation_prev=g_model_rotation;
		}
		else
		{
			g_mouse_rotation_mode=0;
		}
	}
}

void mouse_motion(int x, int y)
{
	if (g_mouse_rotation_mode)
	{
		float y_rot_angle=0.5f*(x-g_mouse_x);
		float x_rot_angle=0.5f*(y-g_mouse_y);
		mat4 M=CalculateRotationMatrixInc(x_rot_angle, y_rot_angle, 3.0f);
		g_model_rotation=M*g_model_rotation_prev;

		glutPostRedisplay();
	}
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutCreateWindow("3D Models");

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
