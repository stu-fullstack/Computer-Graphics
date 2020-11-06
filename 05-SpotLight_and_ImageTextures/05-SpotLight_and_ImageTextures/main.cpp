#include "GL/glew.h"
#include "GL/freeglut.h"
#include "vec.h"
#include <stdlib.h>
#include "mat.h"
#include "GLHelper.h"
#include "Mesh.h"
#include "Camera.h"
#include "ImageLib.h"

#define CMESH_NUM 6
class CObject3D
{
public:
	CMesh mesh;
	color4 base_color;
	mat4 model_matrix;
	GLuint diffuse_texture;
};

GLuint g_GLSL_prog;
int g_model_matrix_loc, g_base_color_loc;

float g_scene_size=10.0f;
CObject3D g_obj[CMESH_NUM];

CCamera g_camera;
float g_camera_step=0.02f*g_scene_size;
int g_mouse_rotation_mode=0;
int g_mouse_x, g_mouse_y;

GLuint g_checkerboard_texture;

GLuint CreateCheckerBoardTexture(void)
{
	GLubyte tex_image[64][64][3];
	const GLubyte color[2]={128, 255};
	GLubyte c;
	int i, j;
	for (j=0; j<64; ++j)
	{
		for (i=0; i<64; ++i)
		{
			if (((i>>3)&1)==((j>>3)&1)) c=color[0];
			else c=color[1];
			tex_image[i][j][0]=c;
			tex_image[i][j][1]=c;
			tex_image[i][j][2]=c;
		}
	}

	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0,
		GL_RGB, GL_UNSIGNED_BYTE, tex_image);
	glGenerateMipmap(GL_TEXTURE_2D);
	GLfloat border_color[4]={0.0f, 1.0f, 0.0f, 1.0f};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	return tex;
}

void init_shaders(void)
{
	g_GLSL_prog=InitShader(
		"../shaders/single_light-single_texture-vs.txt",
		"../shaders/single_light-single_texture-fs.txt");

	g_model_matrix_loc=glGetUniformLocation(g_GLSL_prog, "model_matrix");
	g_base_color_loc=glGetUniformLocation(g_GLSL_prog, "base_color");

	int loc;
	//point light
	loc=glGetUniformLocation(g_GLSL_prog, "ambient_light_color");
	glUniform4f(loc, 0.106f, 0.106f, 0.106f, 1.0f);

	loc=glGetUniformLocation(g_GLSL_prog, "light_color");
	glUniform4f(loc, 0.0f, 0.0f, 0.0f, 1.0f);
	loc=glGetUniformLocation(g_GLSL_prog, "light_position");
	glUniform4f(loc, 1.2f*g_scene_size, 1.0f*g_scene_size, 1.6f*g_scene_size, 1.0f);

	//聚光灯相关属性设置
	glUniform4f(glGetUniformLocation(g_GLSL_prog, "spot_light.position"), g_camera.GetCameraPosition().x, g_camera.GetCameraPosition().y, g_camera.GetCameraPosition().z, 1.0f);	//光源位置，初始在相机位置
	glUniform4f(glGetUniformLocation(g_GLSL_prog, "spot_light.color"), 1.0f, 1.0f, 1.0f, 1.0f);	//探照灯光源强度
	glUniform4f(glGetUniformLocation(g_GLSL_prog, "spot_light.direction"), 0.0f, -1.0f, 0.0f, 1.0f);	//探照灯光源初始方向，和照相机初始化有关，初始值是 normalize(at - eye)
	glUniform1f(glGetUniformLocation(g_GLSL_prog, "spot_light.cut_off_angle"), 15 * DegreesToRadians);	//探照灯光源裁剪角度
	glUniform1f(glGetUniformLocation(g_GLSL_prog, "spot_light.exponent"), 1.32f);	//聚光指数
	glUniform4f(glGetUniformLocation(g_GLSL_prog, "spot_light.ambient"), 0.0f, 0.0f, 0.0f, 1.0f);	//环境光强度
	glUniform4f(glGetUniformLocation(g_GLSL_prog, "spot_light.diffuse"), 1.0f, 1.0f, 1.0f, 1.0f);	//漫反射强度
	glUniform4f(glGetUniformLocation(g_GLSL_prog, "spot_light.specular"), 0.4f, 0.4f, 0.4f, 1.0f);	//镜面反射强度
	glUniform1f(glGetUniformLocation(g_GLSL_prog, "spot_light.a0"), 1.0f);	//衰减系数
	glUniform1f(glGetUniformLocation(g_GLSL_prog, "spot_light.a1"), 0.01f);	
	glUniform1f(glGetUniformLocation(g_GLSL_prog, "spot_light.a2"), 0.005f);



	loc=glGetUniformLocation(g_GLSL_prog, "diffuse_texture");
	glUniform1i(loc, 0);
}

void init_scene(void)
{
	float s=0.1f*g_scene_size;
	float tetra_s=2.0f*s;
	point3 tetra_vertices[4]={
		point3(-0.5f*tetra_s, -0.28868f*tetra_s, 0.0f),
		point3( 0.5f*tetra_s, -0.28868f*tetra_s, 0.0f),
		point3(0.0f, 0.57735f*tetra_s, 0.0f),
		point3(0.0f, 0.0f, 0.81650f*tetra_s),
	};

	
	g_obj[0].mesh.CreateRect(g_scene_size, g_scene_size, 32, 32, 10.0f, 10.0f);
	g_obj[1].mesh.CreateGasket3D(tetra_vertices, 4);
	g_obj[2].mesh.CreateBlock(2.0f * s, 1.5f * s, 1.0f * s, 4.0f, 3.0f, 2.0f);
	g_obj[3].mesh.CreateSphere(0.7f * s, 32, 32, 1.0f, 1.0f);
	g_obj[4].mesh.CreateCone(1.0f, 1.6f, 64, 64, 16, 1.0f, 1.0f);
	g_obj[5].mesh.CreateCylinder(1.0f, 1.2f, 64, 64, 16, 1.0f, 1.0f);

	g_obj[0].base_color = color4(1.0f);
	g_obj[1].base_color = color4(1.0f);
	g_obj[2].base_color = color4(1.0f);
	g_obj[3].base_color = color4(1.0f);
	g_obj[4].base_color = color4(1.0f);
	g_obj[5].base_color = color4(1.0f);

	float d = 0.2f * g_scene_size;
	g_obj[0].model_matrix = mat4(1.0f);
	g_obj[1].model_matrix = Translate(-1.4 * d, -0.5f * d, 0.0f);
	g_obj[2].model_matrix = Translate(1.2 * d, -1.2 * d, 0.5f * s);
	g_obj[3].model_matrix = Translate(1.2 * d, 1.2 * d, 0.7f * s);
	g_obj[4].model_matrix = Translate(-0.8f * d, 1.2f * d, 0.5f * s) * RotateX(60.0f);
	g_obj[5].model_matrix = Translate(0.0f * d, 0.0f * d, 0.8f * s);


//	g_checkerboard_texture=CreateCheckerBoardTexture();
	g_obj[0].diffuse_texture=LoadTexture2DFromFile(
		"../textures/stone01.jpg");
	g_obj[1].diffuse_texture=0;
	g_obj[2].diffuse_texture=LoadTexture2DFromFile(
		"../textures/wood01.jpg");
	g_obj[3].diffuse_texture=LoadTexture2DFromFile(
		"../textures/earth.jpg");	
	g_obj[4].diffuse_texture=LoadTexture2DFromFile(
		"../textures/rock03.jpg");	
	g_obj[5].diffuse_texture=LoadTexture2DFromFile(
		"../textures/stake.jpg");
}

void init(void)
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	g_camera.Init(
		vec3(0.0f, 0.8f * g_scene_size, 0.1f* g_scene_size),
		vec3(0.0f, 0.0f, 0.1f * g_scene_size),
		vec3(0.0f, 0.0f, 1.0f));

	init_shaders();
	init_scene();

	glClearColor(0.0, 0.0, 0.0, 0.0);
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
	int loc=glGetUniformLocation(g_GLSL_prog, "view_matrix");
	glUniformMatrix4fv(loc, 1, GL_TRUE, M);

	//更新探照灯光源的位置和方向
	glUniform4f(glGetUniformLocation(g_GLSL_prog, "spot_light.position"), g_camera.GetCameraPosition().x, g_camera.GetCameraPosition().y, g_camera.GetCameraPosition().z, 1.0f);

	for (int i=0; i < CMESH_NUM; i++)
	{
		glUniformMatrix4fv(g_model_matrix_loc, 1, GL_TRUE, 
			g_obj[i].model_matrix);

		loc=glGetUniformLocation(g_GLSL_prog, "normal_matrix");
		M33=Normal(g_obj[i].model_matrix);
		glUniformMatrix3fv(loc, 1, GL_TRUE , M33);

		loc=glGetUniformLocation(g_GLSL_prog, "diffuse_reflectivity");
		glUniform4f(loc, 1.0f, 1.0f, 1.0f, 1.0f);
		loc=glGetUniformLocation(g_GLSL_prog, "specular_reflectivity");
		glUniform4f(loc, 0.8f, 0.8f, 1.0f, 1.0f);
		loc=glGetUniformLocation(g_GLSL_prog, "shininess");
		glUniform1f(loc, 128.0f);

		glUniform4fv(g_base_color_loc, 1, g_obj[i].base_color);

		loc=glGetUniformLocation(g_GLSL_prog, "enable_diffuse_texture");
		glUniform1i(loc, g_obj[i].diffuse_texture!=0);

		glBindTexture(GL_TEXTURE_2D, g_obj[i].diffuse_texture);

		g_obj[i].mesh.Draw();
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
float thetax = 0.0f;
float thetay = 0.0f;
vec3 gg = vec3(0.0f, -1.0f, 0.0f);
mat4 ad = mat4(1.0f);
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
		vec3 new_direct = g_camera.GetNewFront();
		glUniform4f(glGetUniformLocation(g_GLSL_prog, "spot_light.direction"), new_direct.x, new_direct.y, new_direct.z, 0.0f);	//探照灯光源方向向量更新 

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

	glutCreateWindow("3D scene with textures loaded from image files");

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
