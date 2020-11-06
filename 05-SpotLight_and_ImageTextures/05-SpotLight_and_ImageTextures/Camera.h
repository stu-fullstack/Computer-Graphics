#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "mat.h"

class CCamera
{
protected:
	//front是相机初始看向位置的向量，front_new是拖动鼠标后相机看向位置的向量
	vec3 P0, front_new,front; // The origin
	vec3 u0, v0, n0;
		// Initial unit vectors in x, y, z directions
	vec3 u1, n1;
		// Unit vectors in x, z directions after applying
		//   the horizontal rotation angle (v1=v0)
	vec3 v, n;
		// Final Unit vectors in y, z directions (u=u1)

	float theta; // Horizontal rotation angle for turning left and right
	float phi;   // Vertical rotation angle for looking up and down

	mat4 qua_rotation = mat4(1.0f);
	void CalculateVectors(void);
	// Calculate unit vectors in x, y, z directions

public:
	CCamera(void);

	void Init(const vec3& eye, const vec3& at, const vec3& up);
	// Initialize the camera
	// eye: (in) Camera position
	// at:  (in) A point on the camera viewing direction
	// up:  (in) Up direction

	void GetViewMatrix(mat4& Mview);
	// Get current view matrix	
	
	vec3 GetCameraPosition();
	// Get current Camera psition
	
	vec3 GetNewFront();	//获得当前相机的指向方向向量
	void MoveForward(float step);
	void MoveLeft(float step);
	void MoveUp(float step);
	void TurnLeft(float angle);
	void LookUp(float angle);
};

#endif
