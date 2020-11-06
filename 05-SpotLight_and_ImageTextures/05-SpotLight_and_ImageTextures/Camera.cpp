#include "Camera.h"

void CCamera::CalculateVectors(void)
// Calculate unit vectors in x, y, z directions
{
	// Apply the horizontal rotation angle
	float a=theta*DegreesToRadians;
	float c=cos(a);
	float s=sin(a);
	u1=c*u0-s*n0;
	n1=s*u0+c*n0;

	// Apply the vertical rotation angle
	a=phi*DegreesToRadians;
	c=cos(a);
	s=sin(a);  
	v=c*v0+s*n1;
	n=-s*v0+c*n1;

	//将照相机看向的位置向量更新
	vec4 new_direct = vec4(front.x, front.y, front.z, 0.0f);
	new_direct = RotateZ(theta)* RotateX(-phi) * new_direct;		//RotateX(-phi)* RotateZ(theta)
	front_new.x = new_direct.x;
	front_new.y = new_direct.y;
	front_new.z = new_direct.z;
	
	printf("new_direct(%f,%f,%f)theta,phi(%f,%f)\n", new_direct.x, new_direct.y, new_direct.z, theta, phi);
}

CCamera::CCamera(void)
{
	theta=0.0f;
	phi=0.0f;
}

void CCamera::Init(const vec3& eye, const vec3& at, const vec3& up)
// Initialize the camera
// eye: (in) Camera position
// at:  (in) A point on the camera viewing direction
// up:  (in) Up direction
{
	P0=eye;
	front = normalize(at - eye);
    n0 = normalize(eye - at);
    u0 = normalize(cross(up,n0));
    v0 = cross(n0,u0);

	u1=u0;
	n1=n0;
	v=v0;
	n=n0;

	theta=0.0f;
	phi=0.0f;

}

void CCamera::GetViewMatrix(mat4& Mview)
// Get current view matrix
{
	vec3 u=u1;

	Mview[0]=vec4(u, -dot(u, P0));
	Mview[1]=vec4(v, -dot(v, P0));
	Mview[2]=vec4(n, -dot(n, P0));
	Mview[3]=vec4(0.0f, 0.0f, 0.0f, 1.0f);
}
vec3 CCamera::GetNewFront()
{
	return front_new;
}
vec3 CCamera::GetCameraPosition()
{
	return P0;
}

void CCamera::MoveForward(float step)
{
	P0-=step*n1;
}

void CCamera::MoveLeft(float step)
{
	P0-=step*u1;
}

void CCamera::MoveUp(float step)
{
	P0+=step*v0;
}

void CCamera::TurnLeft(float angle)
{
	theta+=angle;
	theta=fmod(theta, 360.0f);
	CalculateVectors();
}

void CCamera::LookUp(float angle)
{
	phi+=angle;
	if (phi<-90.0f) phi=-90.0f;
	else if (phi>90.0f) phi=90.0f;
	CalculateVectors();
}
