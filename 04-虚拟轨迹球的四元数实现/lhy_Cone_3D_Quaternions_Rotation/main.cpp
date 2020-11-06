#include "GL/glew.h"
#include "GL/freeglut.h"
#include "vec.h"
#include <stdlib.h>
#include "mat.h"
#include "GLHelper.h"
#include "Mesh.h"

//这是一个演示程序，默认下需要的参数应该合理，本程序没有对输入进行严格的debug。

//四元数
#include "Quaternions.h"

GLuint g_GLSL_prog;
CMesh g_obj;

int g_mouse_rotation_mode=0;	//旋转模式
int g_mouse_x, g_mouse_y;	//屏幕上的鼠标坐标
int g_model_matrix_loc;		//模型矩阵的uniform变量索引(location)
int g_window_width, g_window_height;	//glut窗口的宽高

//在display函数中，鼠标移动一定距离输出的点数是和鼠标DPI有关的。
//本程序中，在屏幕上转动物体的速度是和鼠标移动过程中执行多少次display函数有关的
//现在鼠标的DPI大多是800左右，有些高DPI鼠标也能到几万
//如果没有设置DPI，以800DPI计算，你在屏幕上移动一英寸，会产生800个点，
//由于旋转的起点和终点是实时更新的(意味这很短时间内要更新800次起点)，
//这就导致旋转速度过快(特别是鼠标移动距离越来越大的时候，产生点是累积的，旋转速度也是累积的)。
//在本例中是上述情况
//GLuint mouse_dots_per_inch = 15;	//鼠标DPI控制
//GLuint mouse_move_dots;		//当前移动鼠标一共产生的点数,如果屏幕产生了 mouse_dots_per_inch 个点，则重置计数器

qua quaternion_rotation;			//向量a绕单位旋转轴n旋转theta角度得到b,这个旋转过程由这个四元数表示
mat4 quaternion_rotation_matrix;	//四元数对应的旋转矩阵
vec2 normalized_pos;				//经过归一化后的屏幕坐标
vec3 virtual_trackball_pos;			//归一化坐标映射到虚拟轨迹球后的坐标
vec3 start_pos;						//虚拟轨迹球上的起点
vec3 end_pos;						//虚拟轨迹球上的终点



#define MENU_ITEM_POLYGON_MODE_LINE 10		//切换显示模式，线框  填充
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

	quaternion_rotation_matrix = qua_cast_to_rotation_mat(quaternion_rotation) * quaternion_rotation_matrix;	//这里必须是左乘，因为位置交换后代表的旋转是不同的
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

void MousePosToNormalizedPos(		//鼠标坐标归一化
	float mouse_pos_x, float mouse_pos_y,
	float& normalized_pos_x, float& normalized_pos_y)
{
	normalized_pos_x = 2.0f * mouse_pos_x / g_window_width - 1.0f;
	normalized_pos_y = 2.0f * (g_window_height - mouse_pos_y) / g_window_height - 1.0f;
}

void NormalizedPosToVirtualTrackball(		//归一化坐标映射
	float normalized_pos_x, float normalized_pos_y,	//归一化的坐标 (Xn,Yn)
	float &virtual_trackball_x, float& virtual_trackball_y, float& virtual_trackball_z)		//映射到虚拟轨迹球的坐标 (Xs,Ys,Zs)
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

void GetQuaternionOfRotation(		//获得旋转的四元数表示，存入 quaternion_rotation 全局变量
	float start_pos_x, float start_pos_y, float start_pos_z,	//虚拟轨迹球上的起点
	float end_pos_x, float end_pos_y, float end_pos_z,			//虚拟轨迹球上的终点
	qua & quaternion_rotation	)		//旋转变换的四元数
{
	vec3 v1 = vec3(start_pos_x, start_pos_y, start_pos_z);
	vec3 v2 = vec3(end_pos_x, end_pos_y, end_pos_z);
	float theta = acos(dot(v1, v2) / (length(v1) * length(v2)));	//这里长度其实是1，不用做除法也行
	quaternion_rotation = qua(
		sin(theta / 2)*im_cross(im(end_pos_x, end_pos_y, end_pos_z), im(start_pos_x, start_pos_y, start_pos_z)),		//叉乘得到法向量，右手定则需要交换位置做叉乘
		cos(theta / 2)*im_dot(im(start_pos_x, start_pos_y, start_pos_z), im(end_pos_x, end_pos_y, end_pos_z))		//点乘得到旋转变换的四元数的实部
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
			//获得轨迹球上的起点
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
			//更新轨迹球上的起点
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
		//更新鼠标坐标
		g_mouse_x=x;
		g_mouse_y=y;

		//获得轨迹球上的终点
		MousePosToNormalizedPos(g_mouse_x, g_mouse_y, normalized_pos.x, normalized_pos.y);
		NormalizedPosToVirtualTrackball(normalized_pos.x, normalized_pos.y, virtual_trackball_pos.x, virtual_trackball_pos.y, virtual_trackball_pos.z);
		end_pos.x = virtual_trackball_pos.x;
		end_pos.y = virtual_trackball_pos.y;
		end_pos.z = virtual_trackball_pos.z;
		
		//将当前旋转记录到 quaternion_rotation
		GetQuaternionOfRotation(start_pos.x, start_pos.y, start_pos.z, end_pos.x, end_pos.y, end_pos.z, quaternion_rotation);

		//更新轨迹球上的起点
		glutPostRedisplay();	//重画以显示当前的旋转

	}
	/*
	if (mouse_move_dots % mouse_dots_per_inch == 0) {		//灵敏度，调节鼠标转动物体的速度。因为如果不设置，由于鼠标移动过程中  两个点相差太小，旋转角度特别小，导致旋转的速度很慢
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
