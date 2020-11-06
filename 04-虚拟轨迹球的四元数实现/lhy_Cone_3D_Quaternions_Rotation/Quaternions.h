//////////////////////////////////////////////////////////////////////////////
//
//  --- quaternions.h ---
//	û��ʹ��C++ template
//  �����������Ͷ�����Ϊ  float
//	����im��re��Ϊ�˷���һЩ���㣨��Ҫ��im��
//	���.h��������ȫ����������  mat.h  ��ʱ��ԭ�����õĹ��߾�ֱ������
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __QUATERNIONS_H__
#define __QUATERNIONS_H__

#include <math.h>
#include <mat.h>

//  Defined constant for when numbers are too small to be used in the
//    denominator of a division operation.  This is only used if the
//    DEBUG macro is defined.
const float  DivideByZero = float(1.0e-07);

struct im		//��Ԫ���鲿, �����vec.h��vec3����ͬ��
{
	float x;
	float y;
	float z;

	//
	//  --- Constructors and Destructors ---
	//
	im(float x, float y, float z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}
	//
	//  --- Operator overloading ---
	//
	im operator - () const
	{
		return im(-x, -y, -z);
	}

	im operator + (const im& v) const
	{
		return im(x + v.x, y + v.y, z + v.z);
	}

	im operator - (const im& v) const
	{
		return im(x - v.x, y - v.y, z - v.z);
	}

	im operator * (const float s) const
	{
		return im(s * x, s * y, s * z);
	}

	im operator * (const im& v) const
	{
		return im(x * v.x, y * v.y, z * v.z);
	}

	friend im operator * (const float s, const im& v)
	{
		return v * s;
	}

};

inline
float im_dot(const im& a, const im& b) {		//�鲿�������ֵĵ��
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
inline
im im_cross(const im& a, const im& b)		//�鲿�����Ĳ��
{
	return im(a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x);
}


struct re		//��Ԫ��ʵ����Ϊ�˴���һ����Ԫ���Ľṹ������Ĺ��캯��Ҳû�����õ�һ���������档�����������ûʲô������
{
	float w;
	//
	//  --- Constructors and Destructors ---
	//
	re(float w)
	{
		this->w = w;
	}
};

struct qua		//��Ԫ��(����ʹ��  xyzw  ��ʽ)
{
	float x;
	float y;
	float z;
	float w;

public:
	//
	//  --- Constructors and Destructors ---
	//
	qua(float s = 0.0f) :x(s), y(s), z(s), w(s) {}	//Ĭ�Ϲ��캯������Ҳ��֪����ʼ��Ϊʲô�ã��Ǿ�0��
	qua(float x, float y, float z, float w)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}
	qua(const im& i, const float& w)
	{
		this->x = i.x;
		this->y = i.y;
		this->z = i.z;
		this->w = w;
	}
	qua(const im& i, const re& r)
	{
		this->x = i.x;
		this->y = i.y;
		this->z = i.z;
		this->w = r.w;
	}

	//
	//  --- Operator overloading ---
	//
	qua operator - () const
	{
		return qua(-x, -y, -z, -w);
	}
	qua operator + (const qua& b) const
	{
		return qua(x + b.x, y + b.y, z + b.z, w + b.w);
	}
	qua operator - (const qua& b) const
	{
		return qua(x - b.x, y - b.y, z - b.z, w - b.w);
	}
	qua operator * (const float s) const
	{
		return qua(s * x, s * y, s * z, s * w);
	}

	friend qua operator * (const float s, const qua& v)
	{
		return v * s;
	}
	qua operator * (const qua& b) const		//������Ԫ���˷�   a*b
	{
		float q0 = w;		//a��ʵ��
		float p0 = b.w;		//b��ʵ��
		im q_ = im(x, y, z);	//a���鲿
		im p_ = im(b.x, b.y, b.z);	//b���鲿

		return qua((q0 * p_ + p0 * q_ + im_cross(q_, p_)), p0 * q0 - im_dot(q_, p_));
	}
	qua operator / (const float& s) const
	{
#ifdef DEBUG
		if (fabs(s) < DivideByZero) {
			printf("Division by zero\n");
			return qua();
		}
#endif // DEBUG

		float r = float(1.0) / s;
		return *this * r;

	}
};

inline
float qua_dot(const qua& a, const qua& b) {		//��Ԫ��a��b�ĵ��
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}
inline
float qua_length(const qua& a) {		//��Ԫ��a��ģ��
	return sqrt(qua_dot(a, a));
}
inline
qua qua_normalize(const qua& a) {		//��λ����Ԫ��
	return a / qua_length(a);
}
inline
qua qua_conjugation(const qua& a)		//������a�������Ԫ��
{
	return qua(im(-a.x, -a.y, -a.z), a.w);
}
inline
qua qua_inverse(const qua& a)			//������Ԫ��a����
{
	return qua_conjugation(a) / qua_length(a);
}
mat4 qua_cast_to_rotation_mat(const qua& a)	//��һ����λ��Ԫ��( ||a|| = 1 )ת��Ϊ��ת����
{
#ifdef DEBUG	//����ǲ��ǵ�λ��Ԫ��
	if (!(qua_length(a) - 1.0f) < DivideByZero)
	{
		printf("input is not a std quaternion.(req:length is 1)\n");
		return mat4();
	}
#endif // DEBUG
	mat4 rotation = mat4(1.0f);
	rotation[0][0] = 1 - 2 * (a.y * a.y + a.z * a.z);
	rotation[0][1] = 2 * (a.x * a.y + a.w * a.z);
	rotation[0][2] = 2 * (a.x * a.z - a.w * a.y);
	rotation[1][0] = 2 * (a.x * a.y - a.w * a.z);
	rotation[1][1] = 1 - 2 * (a.x * a.x + a.z * a.z);
	rotation[1][2] = 2 * (a.w * a.x + a.y * a.z);
	rotation[2][0] = 2 * (a.w * a.y + a.x * a.z);
	rotation[2][1] = 2 * (a.y * a.z - a.w * a.x);
	rotation[2][2] = 1 - 2 * (a.x * a.x + a.y * a.y);
	return rotation;
}

qua euler_angle_to_qua(float pitch, float yaw, float roll)	//ŷ���Ǳ�ʾ����תתΪ��Ԫ��
{
	float sinp = sin(pitch);
	float siny = sin(yaw);
	float sinr = sin(roll);
	float cosp = cos(pitch);
	float cosy = cos(yaw);
	float cosr = cos(roll);

	return qua(sinr * cosp * cosy - cosr * sinp * siny, cosr * sinp * cosy + sinr * cosp * siny, cosr * cosp * siny - sinr * sinp * cosy, cosr * cosp * cosy + sinr * sinp * siny);

}
#endif	 //	__QUATERNIONS_H__