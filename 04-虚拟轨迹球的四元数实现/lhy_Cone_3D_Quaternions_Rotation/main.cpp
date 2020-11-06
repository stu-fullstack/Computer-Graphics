#include "GL/glew.h"
#include "GL/freeglut.h"
#include "vec.h"
#include <stdlib.h>
#include "mat.h"
#include "GLHelper.h"
#include "Mesh.h"

//����һ����ʾ����Ĭ������Ҫ�Ĳ���Ӧ�ú���������û�ж���������ϸ��debug��

//��Ԫ��
#include "Quaternions.h"

GLuint g_GLSL_prog;
CMesh g_obj;

int g_mouse_rotation_mode=0;	//��תģʽ
int g_mouse_x, g_mouse_y;	//��Ļ�ϵ��������
int g_model_matrix_loc;		//ģ�;����uniform��������(location)
int g_window_width, g_window_height;	//glut���ڵĿ��

//��display�����У�����ƶ�һ����������ĵ����Ǻ����DPI�йصġ�
//�������У�����Ļ��ת��������ٶ��Ǻ�����ƶ�������ִ�ж��ٴ�display�����йص�
//��������DPI�����800���ң���Щ��DPI���Ҳ�ܵ�����
//���û������DPI����800DPI���㣬������Ļ���ƶ�һӢ�磬�����800���㣬
//������ת�������յ���ʵʱ���µ�(��ζ��ܶ�ʱ����Ҫ����800�����)��
//��͵�����ת�ٶȹ���(�ر�������ƶ�����Խ��Խ���ʱ�򣬲��������ۻ��ģ���ת�ٶ�Ҳ���ۻ���)��
//�ڱ��������������
//GLuint mouse_dots_per_inch = 15;	//���DPI����
//GLuint mouse_move_dots;		//��ǰ�ƶ����һ�������ĵ���,�����Ļ������ mouse_dots_per_inch ���㣬�����ü�����

qua quaternion_rotation;			//����a�Ƶ�λ��ת��n��תtheta�Ƕȵõ�b,�����ת�����������Ԫ����ʾ
mat4 quaternion_rotation_matrix;	//��Ԫ����Ӧ����ת����
vec2 normalized_pos;				//������һ�������Ļ����
vec3 virtual_trackball_pos;			//��һ������ӳ�䵽����켣��������
vec3 start_pos;						//����켣���ϵ����
vec3 end_pos;						//����켣���ϵ��յ�



#define MENU_ITEM_POLYGON_MODE_LINE 10		//�л���ʾģʽ���߿�  ���
#define MENU_ITEM_POLYGON_MODE_FILL 11

void init_shaders(void)
{
	g_GLSL_prog=InitShader(
		"..\\shaders\\simple3D-cone-quaternions-rotion-vs.txt",
		"..\\shaders\\simple3D-cone-quaternions-rotion-fs.txt");

	mat4 M;
	int loc=glGetUniformLocation(g_GLSL_prog, "view_matrix");
	glUniformMatrix4fv(loc, 1, GL_TRUE, M);
	g_model_matrix_loc=glGetUniformLocation(g_GLSL_prog, "model_matrix");
}

void init_scene(void)
{
	g_obj.CreateCone(0.8f, 0.8f, 32, 32, 8);
}

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

void init(void)
{
	init_shaders();
	init_scene();

	glClearColor(1.0, 1.0, 1.0, 0.0);
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

	quaternion_rotation_matrix = qua_cast_to_rotation_mat(quaternion_rotation) * quaternion_rotation_matrix;	//�����������ˣ���Ϊλ�ý�����������ת�ǲ�ͬ��
	glUniformMatrix4fv(g_model_matrix_loc, 1, GL_TRUE, Translate(0.0f, 0.0f, -2.0f) * quaternion_rotation_matrix);

	g_obj.Draw();

	glFlush();
	glutSwapBuffers();
}

void reshape(int w, int h)
{
	g_window_width = w;
	g_window_height = h;

	glViewport(0, 0, w, h);

	glUseProgram(g_GLSL_prog);
	int loc=glGetUniformLocation(g_GLSL_prog, "projection_matrix");
	mat4 M;
	M=Perspective(60.0f, (float)w/(float)h, 0.1f, 4.0f);
	glUniformMatrix4fv(loc, 1, GL_TRUE, M);
}

void MousePosToNormalizedPos(		//��������һ��
	float mouse_pos_x, float mouse_pos_y,
	float& normalized_pos_x, float& normalized_pos_y)
{
	normalized_pos_x = 2.0f * mouse_pos_x / g_window_width - 1.0f;
	normalized_pos_y = 2.0f * (g_window_height - mouse_pos_y) / g_window_height - 1.0f;
}

void NormalizedPosToVirtualTrackball(		//��һ������ӳ��
	float normalized_pos_x, float normalized_pos_y,	//��һ�������� (Xn,Yn)
	float &virtual_trackball_x, float& virtual_trackball_y, float& virtual_trackball_z)		//ӳ�䵽����켣������� (Xs,Ys,Zs)
{
	float r = sqrt((double)normalized_pos_x * normalized_pos_x + (double)normalized_pos_y * normalized_pos_y);
	if (r <= 1)
	{
		virtual_trackball_x = normalized_pos_x;
		virtual_trackball_y = normalized_pos_y;
		virtual_trackball_z = sqrt(1 - (double)r*r);
	}
	else
	{
		virtual_trackball_x = normalized_pos_x / r;
		virtual_trackball_y = normalized_pos_y / r;
		virtual_trackball_z = 0;
	}
}

void GetQuaternionOfRotation(		//�����ת����Ԫ����ʾ������ quaternion_rotation ȫ�ֱ���
	float start_pos_x, float start_pos_y, float start_pos_z,	//����켣���ϵ����
	float end_pos_x, float end_pos_y, float end_pos_z,			//����켣���ϵ��յ�
	qua & quaternion_rotation	)		//��ת�任����Ԫ��
{
	vec3 v1 = vec3(start_pos_x, start_pos_y, start_pos_z);
	vec3 v2 = vec3(end_pos_x, end_pos_y, end_pos_z);
	float theta = acos(dot(v1, v2) / (length(v1) * length(v2)));	//���ﳤ����ʵ��1������������Ҳ��
	quaternion_rotation = qua(
		sin(theta / 2)*im_cross(im(end_pos_x, end_pos_y, end_pos_z), im(start_pos_x, start_pos_y, start_pos_z)),		//��˵õ������������ֶ�����Ҫ����λ�������
		cos(theta / 2)*im_dot(im(start_pos_x, start_pos_y, start_pos_z), im(end_pos_x, end_pos_y, end_pos_z))		//��˵õ���ת�任����Ԫ����ʵ��
	);
	quaternion_rotation = qua_normalize(quaternion_rotation);
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
			//��ù켣���ϵ����
			MousePosToNormalizedPos(g_mouse_x, g_mouse_y, normalized_pos.x, normalized_pos.y);
			NormalizedPosToVirtualTrackball(normalized_pos.x, normalized_pos.y, virtual_trackball_pos.x, virtual_trackball_pos.y, virtual_trackball_pos.z);
			start_pos.x = virtual_trackball_pos.x;
			start_pos.y = virtual_trackball_pos.y;
			start_pos.z = virtual_trackball_pos.z;
		}
		else
		{
			g_mouse_rotation_mode = 0;

			g_mouse_x = x;
			g_mouse_y = y;
			//���¹켣���ϵ����
			MousePosToNormalizedPos(g_mouse_x, g_mouse_y, normalized_pos.x, normalized_pos.y);
			NormalizedPosToVirtualTrackball(normalized_pos.x, normalized_pos.y, virtual_trackball_pos.x, virtual_trackball_pos.y, virtual_trackball_pos.z);
			//quaternion_rotation_matrix = mat4();
			start_pos.x = virtual_trackball_pos.x;
			start_pos.y = virtual_trackball_pos.y;
			start_pos.z = virtual_trackball_pos.z;

		}
	}
}
void mouse_motion(int x, int y)
{
	if (g_mouse_rotation_mode)
	{
		//mouse_move_dots++;
		//�����������
		g_mouse_x=x;
		g_mouse_y=y;

		//��ù켣���ϵ��յ�
		MousePosToNormalizedPos(g_mouse_x, g_mouse_y, normalized_pos.x, normalized_pos.y);
		NormalizedPosToVirtualTrackball(normalized_pos.x, normalized_pos.y, virtual_trackball_pos.x, virtual_trackball_pos.y, virtual_trackball_pos.z);
		end_pos.x = virtual_trackball_pos.x;
		end_pos.y = virtual_trackball_pos.y;
		end_pos.z = virtual_trackball_pos.z;
		
		//����ǰ��ת��¼�� quaternion_rotation
		GetQuaternionOfRotation(start_pos.x, start_pos.y, start_pos.z, end_pos.x, end_pos.y, end_pos.z, quaternion_rotation);

		//���¹켣���ϵ����
		glutPostRedisplay();	//�ػ�����ʾ��ǰ����ת

	}
	/*
	if (mouse_move_dots % mouse_dots_per_inch == 0) {		//�����ȣ��������ת��������ٶȡ���Ϊ��������ã���������ƶ�������  ���������̫С����ת�Ƕ��ر�С��������ת���ٶȺ���
		start_pos.x = end_pos.x;
		start_pos.y = end_pos.y;
		start_pos.z = end_pos.z;

		mouse_move_dots = 0;
	}*/
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutCreateWindow("3D Cone with mouse rotation by virtual trackball on quaternions");

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
