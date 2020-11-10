#include "GL/glew.h"
#include "GL/freeglut.h"
#include "vec.h"
#include <stdlib.h>
#include "mat.h"
#include "GLHelper.h"
#include "Mesh.h"
#include "Camera.h"
#include "ImageLib.h"

using namespace std;
#define CMESH_NUM 7
class CObject3D
{
public:
	CMesh mesh;
	color4 base_color;
	mat4 model_matrix;
	GLuint diffuse_texture;
	GLboolean isEnvirmomentObj;
};

GLuint g_GLSL_prog;
//GLuint g_cube_GLSL_prog;
 
float g_scene_size=100.0f;
CObject3D g_obj[CMESH_NUM];

CCamera g_camera;
float g_camera_step=0.6f*g_scene_size;
int g_mouse_rotation_mode=0;
int g_mouse_x, g_mouse_y;

void init_shaders(void)
{
	//InitShader 里面默认启用了  glUseProgram(<当前>);

	g_GLSL_prog=InitShader(
		"../shaders/single_light-single_texture-vs.txt",
		"../shaders/single_light-single_texture-fs.txt");

	glUseProgram(g_GLSL_prog);
	
	int loc;
	//point light
	loc=glGetUniformLocation(g_GLSL_prog, "ambient_light_color");
	glUniform4f(loc, 1.0f, 1.0f, 1.0f, 1.0f);

	loc=glGetUniformLocation(g_GLSL_prog, "light_color");
	glUniform4f(loc, 1.0f, 1.0f, 1.0f, 1.0f);
	loc=glGetUniformLocation(g_GLSL_prog, "light_position");
	glUniform4f(loc, 45.0f * g_scene_size, -45.0f * g_scene_size, 45.0f*g_scene_size, 1.0f);

	vec3 init_camera_pos = g_camera.GetCameraPosition();
	vec3 init_LookAt_pos = g_camera.GetLookAtPoint();
	vec3 init_direct = init_LookAt_pos - init_camera_pos;
	//聚光灯相关属性设置
	glUniform4f(glGetUniformLocation(g_GLSL_prog, "spot_light.position"), init_camera_pos.x, init_camera_pos.y, init_camera_pos.z, 1.0f);	//光源位置，初始在相机位置
	glUniform4f(glGetUniformLocation(g_GLSL_prog, "spot_light.color"), 1.0f, 0.0f, 0.0f, 1.0f);	//探照灯光源强度
	glUniform4f(glGetUniformLocation(g_GLSL_prog, "spot_light.direction"), init_direct.x, init_direct.y, init_direct.z, 1.0f);	//探照灯光源初始方向，和照相机初始化有关，初始值是 normalize(at - eye)
	glUniform1f(glGetUniformLocation(g_GLSL_prog, "spot_light.cut_off_angle"), 10 * DegreesToRadians);	//探照灯光源裁剪角度
	glUniform1f(glGetUniformLocation(g_GLSL_prog, "spot_light.exponent"), 0.005f);	//聚光指数
	glUniform4f(glGetUniformLocation(g_GLSL_prog, "spot_light.ambient"), 1.0f, 1.0f, 1.0f, 1.0f);	//环境光强度
	glUniform4f(glGetUniformLocation(g_GLSL_prog, "spot_light.diffuse"), 1.0f, 1.0f, 1.0f, 1.0f);	//漫反射强度
	glUniform4f(glGetUniformLocation(g_GLSL_prog, "spot_light.specular"), 1.0f, 1.0f, 1.0f, 1.0f);	//镜面反射强度
	glUniform1f(glGetUniformLocation(g_GLSL_prog, "spot_light.a0"), 1.0f);	//衰减系数
	glUniform1f(glGetUniformLocation(g_GLSL_prog, "spot_light.a1"), 0.0000002f);	
	glUniform1f(glGetUniformLocation(g_GLSL_prog, "spot_light.a2"), 0.00000001f);

	glUniform1i(glGetUniformLocation(g_GLSL_prog, "diffuse_texture"), 0);
	glUniform1i(glGetUniformLocation(g_GLSL_prog, "skybox"), 1);

	glUniformMatrix4fv(glGetUniformLocation(g_GLSL_prog, "rotateX_90"), 1, GL_FALSE, RotateX(90.0f));
}

void init_scene(void)
{
	float s=0.1f*g_scene_size;
	float tetra_s=65.0f*s;
	point3 tetra_vertices[4]={
		point3(-0.5f*tetra_s, -0.28868f*tetra_s, 0.0f),
		point3( 0.5f*tetra_s, -0.28868f*tetra_s, 0.0f),
		point3(0.0f, 0.57735f*tetra_s, 0.0f),
		point3(0.0f, 0.0f, 0.81650f*tetra_s),
	};
	
	g_obj[0].mesh.CreateRect(108.0f * g_scene_size, 465.0f / 436.0f * 108.0f * g_scene_size, 436, 465, 1.0f, 1.0f);
	g_obj[1].mesh.CreateGasket3D(tetra_vertices, 6);
	g_obj[2].mesh.CreateBlock(150.0f * s, 150.0f * s, 150.0f * s, 1.0f, 1.0f, 1.0f);
	g_obj[3].mesh.CreateSphere(100.0f * s, 64, 64, 1.0f, 1.0f);
	g_obj[4].mesh.CreateCone(60.0f * s, 90.0f * s, 64, 64, 16, 1.0f, 1.0f);
	g_obj[5].mesh.CreateCylinder(60.0f * s, 90.0f * s, 64, 64, 16, 1.0f, 1.0f);
	g_obj[6].mesh.CreateCube(50.0f * g_scene_size, 1.0f);

	g_obj[0].base_color = color4(1.0f);
	g_obj[1].base_color = color4(1.0f);
	g_obj[2].base_color = color4(1.0f);
	g_obj[3].base_color = color4(1.0f);
	g_obj[4].base_color = color4(1.0f);
	g_obj[5].base_color = color4(1.0f);
	g_obj[6].base_color = color4(1.0f);

	float d = 0.4f * g_scene_size;
	float move_z = -35 * d;		//场景整体向下移动的距离
	g_obj[0].model_matrix = Translate(0.0f, 0.0f, move_z)*mat4(1.0f);
	g_obj[1].model_matrix = Translate(-40.0f * d, 0.0f, -25 * d);
	g_obj[2].model_matrix = Translate(-30.0f * d, 0.0f, 65 * d);
	g_obj[3].model_matrix = Translate(60 * d, 0.0f, 35 * d);
	g_obj[4].model_matrix = Translate(-60.0f * d, 50.0f * d, 40.0f * d + move_z)* RotateZ(40.0f) * RotateX(40.0f);
	g_obj[5].model_matrix = Translate(60.0f * d, 70.0f * d, 20.0f * d + move_z);
	g_obj[6].model_matrix = RotateX(90.0f) * mat4(1.0f);

	g_obj[0].diffuse_texture=LoadTexture2DFromFile(
		"../textures/green.jpg");
	g_obj[1].diffuse_texture=0;
	g_obj[2].diffuse_texture = 0;
	g_obj[3].diffuse_texture=LoadTexture2DFromFile(
		"../textures/earth.jpg");	
	g_obj[4].diffuse_texture=LoadTexture2DFromFile(
		"../textures/rock03.jpg");	
	g_obj[5].diffuse_texture=LoadTexture2DFromFile(
		"../textures/stake.jpg");	
	const char *cube_texture[6] = {
		"../textures/Skybox01_left.jpg",
		"../textures/Skybox01_right.jpg",		
		"../textures/Skybox01_up.jpg",
		"../textures/Skybox01_down.jpg",
		"../textures/Skybox01_front.jpg",
		"../textures/Skybox01_back.jpg"
	};
	g_obj[6].diffuse_texture = LoadTextureCubeMapFromFile(cube_texture);
}

void init(void)
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	g_camera.Init(
		vec3(0.0f, 465 * 0.1f * g_scene_size * 1.45f, 0.5f* g_scene_size),
		vec3(0.0f, 0.0f, 0.5f * g_scene_size),
		vec3(0.0f, 0.0f, 1.0f));

	init_shaders();
	init_scene();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(g_GLSL_prog);

	mat4 M;
	mat3 M33;
	g_camera.GetViewMatrix(M);

	//更新探照灯光源的位置
	glUniform4f(glGetUniformLocation(g_GLSL_prog, "spot_light.position"), g_camera.GetCameraPosition().x, g_camera.GetCameraPosition().y, g_camera.GetCameraPosition().z, 1.0f);

	for (int i=0; i < CMESH_NUM; i++)	//这个循环没有包括立方体纹理
	{
		glUniformMatrix4fv(glGetUniformLocation(g_GLSL_prog, "model_matrix"), 1, GL_TRUE, g_obj[i].model_matrix);
		glUniformMatrix4fv(glGetUniformLocation(g_GLSL_prog, "view_matrix"), 1, GL_TRUE, M);

		M33 = Normal(g_obj[i].model_matrix);
		glUniformMatrix4fv(glGetUniformLocation(g_GLSL_prog, "normal_matrix"), 1, GL_TRUE, M33);

		glUniform4f(glGetUniformLocation(g_GLSL_prog, "diffuse_reflectivity"), 1.0f, 1.0f, 1.0f, 1.0f);
		glUniform4f(glGetUniformLocation(g_GLSL_prog, "specular_reflectivity"), 0.8f, 0.8f, 1.0f, 1.0f);
		glUniform1f(glGetUniformLocation(g_GLSL_prog, "shininess"), 128.0f);
		
		glUniform4fv(glGetUniformLocation(g_GLSL_prog, "base_color"), 1, g_obj[i].base_color);

		glUniform1i(glGetUniformLocation(g_GLSL_prog, "enable_diffuse_texture"), g_obj[i].diffuse_texture != 0);

		if(i != CMESH_NUM - 1)
			glUniform1i(glGetUniformLocation(g_GLSL_prog, "isEnvirmomentObj"), false);
		else
			glUniform1i(glGetUniformLocation(g_GLSL_prog, "isEnvirmomentObj"), true);

		if( i != 2)	glUniform1i(glGetUniformLocation(g_GLSL_prog, "isBlockObj"), false);
		else glUniform1i(glGetUniformLocation(g_GLSL_prog, "isBlockObj"), true);

		if (i != CMESH_NUM - 1)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, g_obj[i].diffuse_texture);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_CUBE_MAP, g_obj[CMESH_NUM - 1].diffuse_texture);
			g_obj[i].mesh.Draw();
		}
		else
		{
			glDepthFunc(GL_LEQUAL);

			glUniformMatrix4fv(glGetUniformLocation(g_GLSL_prog, "view_matrix"), 1, GL_TRUE, removeTranslateFromMatrix2(M));

			g_obj[i].mesh.Draw();

			glDepthFunc(GL_LESS);
		}
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
		0.01f*g_scene_size, 200.0f*g_scene_size);
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

		//更新探照灯光源方向，和照相机看向的位置相同
		glUniform4f(glGetUniformLocation(g_GLSL_prog, "spot_light.direction"), g_camera.GetNewFront().x, g_camera.GetNewFront().y, g_camera.GetNewFront().z, 0.0f);	//探照灯光源方向向量更新 

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
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutCreateWindow("3D scene with cube texture and enviroment influence");

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
