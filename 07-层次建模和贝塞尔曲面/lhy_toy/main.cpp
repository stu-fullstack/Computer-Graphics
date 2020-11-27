#include "GL/glew.h"
#include "GL/freeglut.h"
#include "vec.h"
#include <stdlib.h>
#include "mat.h"
#include "GLHelper.h"
#include "Mesh.h"
#include "Camera.h"
#include "ImageLib.h"
#include <stack>


#define MENU_ITEM_POLYGON_MODE_LINE 10
#define MENU_ITEM_POLYGON_MODE_FILL 11
void main_menu_func(int menu_id)
{
}
void polygon_mode_selection_menu_func(int menu_id)
{
	switch (menu_id)
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

class CObject3D
{
public:
	CMesh *pmesh; // Pointer to the mesh
	mat4 model_matrix;
	color4 base_color;
	GLuint diffuse_texture;

	mat4 M_from_parent; // Matrix relative to the parent
	mat4 M_instance;    // Instance matrix
	CObject3D *p_child;   // Pointer to the first child
	CObject3D *p_sibling; // Pointer to the next sibling
};

GLuint g_GLSL_prog;
int g_model_matrix_loc, g_base_color_loc;

float g_scene_size=10.0f;

enum {
	OBJECT_GROUND=0,
	OBJECT_TOY_PLATFORM,
	OBJECT_TOY_BODY,
	OBJECT_TOY_AXLE,
	OBJECT_TOY_SLICE_ONE,
	OBJECT_TOY_SLICE_TWO,
	OBJECT_TOY_SLICE_THREE,
	OBJECT_TEAPOT,
	OBJECT_TEACUP,
	OBJECT_TEASPOON,
	NUM_OBJECTS
};

enum {
	MESH_GROUND=0,
	MESH_TOY_PLATFORM,
	MESH_TOY_BODY,
	MESH_TOY_AXLE,
	MESH_TOY_SLICE,
	MESH_TEAPOT,
	MESH_TEACUP,
	MESH_TEASPOON,
	NUM_MESHES
};

enum {
	JOINT_ANGLE_GROUND_TO_PLATFORM =0,
	JOINT_ANGLE_PLATFORM_TO_BODY,
	JOINT_ANGLE_LOWER_AXLE_TO_SLICES,
	JOINT_ANGLE_UP_AXLE_TO_SLICES,
	NUM_JOINT_ANGLES
};

CObject3D g_obj[NUM_OBJECTS];
CMesh g_obj_mesh[NUM_MESHES];
float g_joint_angles[NUM_JOINT_ANGLES]={0.0f, 0.0f, 0.0f, 0.0f};
float g_platform_height=0.05f, g_platform_width= g_scene_size * 0.6;
float g_body_radius=0.5f;
float g_axle_height=0.8f, g_axle_radius =0.04f;
float g_slice_radius=0.4f, g_slice_height=0.2f, g_slice_width= 0.05f, g_slice_length= 0.1f;
float g_wheel_radius = 0.6f, g_wheel_width = 0.5f, g_wheel_height = 1.0f;
float g_fans_separation_dist_half=0.1f;

CCamera g_camera;
float g_camera_step=0.01f*g_scene_size;
int g_mouse_rotation_mode=0;
int g_mouse_x, g_mouse_y;

std::stack <mat4> g_matrix_stack;

void TraverseObjTree(CObject3D *p_obj, mat4& current_matrix)
{
	if (p_obj==NULL) return;
	g_matrix_stack.push(current_matrix);
	current_matrix=current_matrix*p_obj->M_from_parent;
	p_obj->model_matrix=current_matrix*p_obj->M_instance;
	if (p_obj->p_child!=NULL)
		TraverseObjTree(p_obj->p_child, current_matrix);
	current_matrix=g_matrix_stack.top();
	g_matrix_stack.pop();
	if (p_obj->p_sibling!=NULL)
		TraverseObjTree(p_obj->p_sibling, current_matrix);
}

void AnimateRobotArm(void)
{
	//PLATFORM
	g_obj[OBJECT_TOY_PLATFORM].M_from_parent =
		Translate(0.0f, 0.0f, 1.0f * g_platform_height) *
		RotateZ(g_joint_angles[JOINT_ANGLE_GROUND_TO_PLATFORM]);

	// BODY
	g_obj[OBJECT_TOY_BODY].M_from_parent =
		Translate(0.0f, 0.0f, 1.0f * g_platform_height + g_body_radius + 1.0f) *
		RotateZ(g_joint_angles[JOINT_ANGLE_PLATFORM_TO_BODY]);

	// AXLE
	g_obj[OBJECT_TOY_AXLE].M_from_parent =
		Translate(0.0f, 0.0f, g_body_radius)
		* RotateZ(g_joint_angles[JOINT_ANGLE_LOWER_AXLE_TO_SLICES]);

	// SLICE ONE
	g_obj[OBJECT_TOY_SLICE_ONE].M_from_parent =
		Translate(0.0f, 0.0f, g_axle_height)
		* RotateZ(0.0f)
		* RotateZ(g_joint_angles[JOINT_ANGLE_UP_AXLE_TO_SLICES]);

	// SLICE TWO
	g_obj[OBJECT_TOY_SLICE_TWO].M_from_parent =
		Translate(0.0f, 0.0f, g_axle_height)
		* RotateZ(120.0f)
		* RotateZ(g_joint_angles[JOINT_ANGLE_UP_AXLE_TO_SLICES]);

	// SLICE THREE
	g_obj[OBJECT_TOY_SLICE_THREE].M_from_parent =
		Translate(0.0f, 0.0f, g_axle_height)
		* RotateZ(240.0f)
		* RotateZ(g_joint_angles[JOINT_ANGLE_UP_AXLE_TO_SLICES]);
	mat4 current_matrix;
	TraverseObjTree(&g_obj[OBJECT_TOY_PLATFORM], current_matrix);


}

void init_shaders(void)
{
	g_GLSL_prog=InitShader(
		"../shaders/final-vs.txt",
		"../shaders/final-fs.txt");

	g_model_matrix_loc=glGetUniformLocation(g_GLSL_prog, "model_matrix");
	g_base_color_loc=glGetUniformLocation(g_GLSL_prog, "base_color");

	int loc;
	loc=glGetUniformLocation(g_GLSL_prog, "ambient_light_color");
	glUniform4f(loc, 0.4f, 0.4f, 0.4f, 1.0f);
	loc=glGetUniformLocation(g_GLSL_prog, "light_color");
	glUniform4f(loc, 1.0f, 1.0f, 1.0f, 1.0f);
	loc=glGetUniformLocation(g_GLSL_prog, "light_position");
	glUniform4f(loc, 1.2f*g_scene_size, 1.0f*g_scene_size, 1.6f*g_scene_size, 1.0f);

	loc=glGetUniformLocation(g_GLSL_prog, "diffuse_texture");
	glUniform1i(loc, 0);

	glUniform1i(glGetUniformLocation(g_GLSL_prog, "cube_texture"), 1);
}

void init_scene(void)
{
	g_obj_mesh[MESH_GROUND].CreateRect(
		g_scene_size, g_scene_size, 32, 32, 10.0f, 10.0f);
	g_obj_mesh[MESH_TOY_PLATFORM].CreateBlock(
		g_platform_width, g_platform_width, g_platform_height, 
		4.0f, 4.0f, ceilf(4.0f* g_platform_height/ g_platform_width));
	g_obj_mesh[MESH_TOY_BODY].CreateSphere(g_body_radius, 64, 64, 1.0f, 1.0f);
	g_obj_mesh[MESH_TOY_AXLE].CreateCylinder(g_axle_radius, g_axle_height, 32, 32, 64, 1.0f, 1.0f);
	g_obj_mesh[MESH_TOY_SLICE].CreateSphere(g_slice_radius, 64, 64, 1.0f, 1.0f);
	g_obj_mesh[MESH_TEAPOT].CreateBezierObject("../models/teapot.txt", 0.02, 1.0f, 1.0f);		//这里的第二个参数必须能被  1  整除(不然绘制的物体因为没有右边界而有裂缝)。
	g_obj_mesh[MESH_TEACUP].CreateBezierObject("../models/teacup.txt", 0.1, 1.0f, 1.0f);		//0.01精度比较高，耗费时间比较多。
	g_obj_mesh[MESH_TEASPOON].CreateBezierObject("../models/teaspoon.txt", 0.2, 1.0f, 1.0f);	//另外可以使用曲面细分着色器来实现细分。

	g_obj[OBJECT_GROUND].pmesh=&g_obj_mesh[MESH_GROUND];
	g_obj[OBJECT_TOY_PLATFORM].pmesh=&g_obj_mesh[MESH_TOY_PLATFORM];
	g_obj[OBJECT_TOY_BODY].pmesh=&g_obj_mesh[MESH_TOY_BODY];
	g_obj[OBJECT_TOY_AXLE].pmesh=&g_obj_mesh[MESH_TOY_AXLE];
	g_obj[OBJECT_TOY_SLICE_ONE].pmesh=&g_obj_mesh[MESH_TOY_SLICE];
	g_obj[OBJECT_TOY_SLICE_TWO].pmesh=&g_obj_mesh[MESH_TOY_SLICE];
	g_obj[OBJECT_TOY_SLICE_THREE].pmesh=&g_obj_mesh[MESH_TOY_SLICE];
	g_obj[OBJECT_TEAPOT].pmesh=&g_obj_mesh[MESH_TEAPOT];
	g_obj[OBJECT_TEACUP].pmesh=&g_obj_mesh[MESH_TEACUP];
	g_obj[OBJECT_TEASPOON].pmesh=&g_obj_mesh[MESH_TEASPOON];

	for (int i = 0; i < NUM_OBJECTS; i++)
	{
		g_obj[i].base_color = color4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	g_obj[OBJECT_GROUND].diffuse_texture=LoadTexture2DFromFile(
		"../textures/wood01.jpg");
	g_obj[MESH_TOY_PLATFORM].diffuse_texture=LoadTexture2DFromFile(
		"../textures/rock03.jpg");
	g_obj[OBJECT_TOY_BODY].diffuse_texture = LoadTexture2DFromFile(
		"../textures/sphere.jpg");
	g_obj[OBJECT_TOY_AXLE].diffuse_texture = g_obj[OBJECT_TEASPOON].diffuse_texture = LoadTexture2DFromFile(
		"../textures/blackwood.jpg");
	g_obj[OBJECT_TOY_SLICE_ONE].diffuse_texture = g_obj[OBJECT_TEASPOON].diffuse_texture = LoadTexture2DFromFile(
			"../textures/blackwood.jpg");
	g_obj[OBJECT_TOY_SLICE_TWO].diffuse_texture = g_obj[OBJECT_TOY_SLICE_ONE].diffuse_texture;
	g_obj[OBJECT_TOY_SLICE_THREE].diffuse_texture = g_obj[OBJECT_TOY_SLICE_ONE].diffuse_texture;
	g_obj[OBJECT_TEAPOT].diffuse_texture= LoadTexture2DFromFile(
		"../textures/black.jpg");
	g_obj[OBJECT_TEACUP].diffuse_texture= LoadTexture2DFromFile(
		"../textures/flower.jpg");
	g_obj[OBJECT_TEASPOON].diffuse_texture= LoadTexture2DFromFile(
		"../textures/redwood.jpg");


	g_obj[OBJECT_TOY_PLATFORM].M_instance=Translate(0.0f, 0.0f, 1.6f*g_platform_height);
	g_obj[OBJECT_TOY_BODY].M_instance=Translate(0.0f, 0.0f, 1.0f * g_platform_height);
	g_obj[OBJECT_TOY_AXLE].M_instance = Translate(0.0f, 0.0f, 0.0f);
	g_obj[OBJECT_TOY_SLICE_ONE].M_instance = Scale(2.0f, g_slice_width, g_slice_height);
	g_obj[OBJECT_TOY_SLICE_TWO].M_instance =  Scale(2.0f, g_slice_width, g_slice_height);
	g_obj[OBJECT_TOY_SLICE_THREE].M_instance = Scale(2.0f, g_slice_width, g_slice_height);
	g_obj[OBJECT_TEAPOT].model_matrix = Translate(1.6f, 1.1f, 0.6f) * Scale(0.35f, 0.35f, 0.35f);
	g_obj[OBJECT_TEACUP].model_matrix = Translate(1.6f, -0.6f, 1.2f) * Scale(0.6f, 0.6f, 0.6f)*RotateX(90.0f);
	g_obj[OBJECT_TEASPOON].model_matrix = Translate(1.6f, -0.6f, 1.6f) * Scale(0.6f, 0.6f, 0.6f) * RotateX(30.0f) * Rotate(-90.0f, 1.0f , 0.0f, 0.0f);

	g_obj[OBJECT_TOY_PLATFORM].p_sibling=NULL;
	g_obj[OBJECT_TOY_PLATFORM].p_child=&g_obj[OBJECT_TOY_BODY];

	g_obj[OBJECT_TOY_BODY].p_sibling=NULL;
	g_obj[OBJECT_TOY_BODY].p_child=&g_obj[OBJECT_TOY_AXLE];

	g_obj[OBJECT_TOY_AXLE].p_sibling =NULL;
	g_obj[OBJECT_TOY_AXLE].p_child = &g_obj[OBJECT_TOY_SLICE_ONE];

	g_obj[OBJECT_TOY_SLICE_ONE].p_sibling = &g_obj[OBJECT_TOY_SLICE_TWO];
	g_obj[OBJECT_TOY_SLICE_ONE].p_child = NULL;

	g_obj[OBJECT_TOY_SLICE_TWO].p_sibling = &g_obj[OBJECT_TOY_SLICE_THREE];
	g_obj[OBJECT_TOY_SLICE_TWO].p_child = NULL;

	g_obj[OBJECT_TOY_SLICE_THREE].p_sibling = NULL;
	g_obj[OBJECT_TOY_SLICE_THREE].p_child = NULL;

	AnimateRobotArm();
}

void init(void)
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	init_shaders();
	init_scene();

	g_camera.Init(
		vec3(0.1f*g_scene_size, 0.2f*g_scene_size, 0.2f*g_scene_size),
		vec3(0.1f*g_scene_size,              0.0f, 0.2f*g_scene_size),
		vec3(0.0f, 0.0f, 1.0f));
	g_camera.TurnLeft(-26.0f);
	g_camera.LookUp(-25.0f);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	int polygon_mode_selection_menu_id = glutCreateMenu(polygon_mode_selection_menu_func);
	glutAddMenuEntry("Line", MENU_ITEM_POLYGON_MODE_LINE);
	glutAddMenuEntry("Fill", MENU_ITEM_POLYGON_MODE_FILL);

	glutCreateMenu(main_menu_func);
	glutAddSubMenu("Select Polygon Mode", polygon_mode_selection_menu_id);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

}
void display(void)
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(g_GLSL_prog);

	mat4 M;
	mat3 M33;
	g_camera.GetViewMatrix(M);
	int loc=glGetUniformLocation(g_GLSL_prog, "view_matrix");
	glUniformMatrix4fv(loc, 1, GL_TRUE, M);

	for (int i= 0; i<NUM_OBJECTS; i++)
	{
		glUniformMatrix4fv(g_model_matrix_loc, 1, GL_TRUE, 
			g_obj[i].model_matrix);

		loc=glGetUniformLocation(g_GLSL_prog, "normal_matrix");
		M33=Normal(g_obj[i].model_matrix);
		glUniformMatrix3fv(loc, 1, GL_TRUE , M33);

		loc=glGetUniformLocation(g_GLSL_prog, "diffuse_reflectivity");
		glUniform4f(loc, 1.0f, 1.0f, 1.0f, 1.0f);
		loc=glGetUniformLocation(g_GLSL_prog, "specular_reflectivity");
		glUniform4f(loc, 0.5f, 0.5f, 1.0f, 1.0f);
		loc=glGetUniformLocation(g_GLSL_prog, "shininess");
		glUniform1f(loc, 512.0f);

		glUniform4fv(g_base_color_loc, 1, g_obj[i].base_color);

		loc=glGetUniformLocation(g_GLSL_prog, "enable_diffuse_texture");
		glUniform1i(loc, g_obj[i].diffuse_texture!=0);

		glBindTexture(GL_TEXTURE_2D, g_obj[i].diffuse_texture);

		g_obj[i].pmesh->Draw();

	}

	glFlush();
	glutSwapBuffers();
}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);

	glUseProgram(g_GLSL_prog);
	int loc=glGetUniformLocation(g_GLSL_prog, "projection_matrix");
	mat4 M;
	M=Perspective(60.0f, (float)w/(float)h, 
		0.01f*g_scene_size, 4.0f*g_scene_size);
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
		float dx=0.5f*(g_mouse_x-x);
		float dy=0.5f*(g_mouse_y-y);
		g_mouse_x=x;
		g_mouse_y=y;
		g_camera.TurnLeft(dx);
		g_camera.LookUp(dy);
		glutPostRedisplay();
	}
}

void keyboard(unsigned char key, int x, int y)
{
	switch(key)
	{
	case 'w':
	case 'W':
		g_camera.MoveForward(g_camera_step);
		glutPostRedisplay();
		break;
	case 's':
	case 'S':
		g_camera.MoveForward(-g_camera_step);
		glutPostRedisplay();
		break;
	case 'a':
	case 'A':
		g_camera.MoveLeft(g_camera_step);
		glutPostRedisplay();
		break;
	case 'd':
	case 'D':
		g_camera.MoveLeft(-g_camera_step);
		glutPostRedisplay();
		break;
	case 'r':
	case 'R':
		g_camera.MoveUp(g_camera_step);
		glutPostRedisplay();
		break;
	case 'f':
	case 'F':
		g_camera.MoveUp(-g_camera_step);
		glutPostRedisplay();
		break;
	}

	if (key>='0' && key<='4')
	{
		float angle_step=3.0f;
		switch (key)
		{
		case '1':
			g_joint_angles[JOINT_ANGLE_GROUND_TO_PLATFORM] -= angle_step;
			g_joint_angles[JOINT_ANGLE_GROUND_TO_PLATFORM] =
				fmod(g_joint_angles[JOINT_ANGLE_GROUND_TO_PLATFORM], 360.0f);
		case '2':
			g_joint_angles[JOINT_ANGLE_PLATFORM_TO_BODY] -= angle_step;
			g_joint_angles[JOINT_ANGLE_PLATFORM_TO_BODY] =
				fmod(g_joint_angles[JOINT_ANGLE_PLATFORM_TO_BODY], 360.0f);
			break;
		case '3':
			g_joint_angles[JOINT_ANGLE_LOWER_AXLE_TO_SLICES] -= angle_step;
			g_joint_angles[JOINT_ANGLE_LOWER_AXLE_TO_SLICES] =
				fmod(g_joint_angles[JOINT_ANGLE_LOWER_AXLE_TO_SLICES], 360.0f);
			break;

		case '4':
			g_joint_angles[JOINT_ANGLE_UP_AXLE_TO_SLICES] -= angle_step;
			g_joint_angles[JOINT_ANGLE_UP_AXLE_TO_SLICES] =
				fmod(g_joint_angles[JOINT_ANGLE_UP_AXLE_TO_SLICES], 360.0f);
			break;
		}
		AnimateRobotArm();
		glutPostRedisplay();
	}
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutCreateWindow("Toy");

	glewExperimental=GL_TRUE;
	glewInit();

	init();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(mouse_motion);
	glutKeyboardFunc(keyboard);

	glutMainLoop();
	return 0;
}
