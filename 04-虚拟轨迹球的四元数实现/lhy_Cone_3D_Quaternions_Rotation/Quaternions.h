//////////////////////////////////////////////////////////////////////////////
//
//  --- quaternions.h ---
//	没有使用C++ template
//  所以数据类型都设置为  float
//	定义im和re是为了方便一些运算（主要是im）
//	这个.h还不是完全独立，依赖  mat.h  ，时间原因，能用的工具就直接用了
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

struct im		//四元数虚部, 定义和vec.h的vec3是相同的
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
float im_dot(const im& a, const im& b) {		//虚部向量部分的点乘
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
inline
im im_cross(const im& a, const im& b)		//虚部向量的叉乘
{
	return im(a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x);
}


struct re		//四元数实部。为了凑齐一下四元数的结构。下面的构造函数也没她，用的一个标量代替。所以这个东西没什么几儿用
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

struct qua		//四元数(数据使用  xyzw  格式)
{
	float x;
	float y;
	float z;
	float w;

public:
	//
	//  --- Constructors and Destructors ---
	//
	qua(float s = 0.0f) :x(s), y(s), z(s), w(s) {}	//默认构造函数，我也不知道初始化为什么好，那就0吧
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
	qua operator * (const qua& b) const		//定义四元数乘法   a*b
	{
		float q0 = w;		//a的实部
		float p0 = b.w;		//b的实部
		im q_ = im(x, y, z);	//a的虚部
		im p_ = im(b.x, b.y, b.z);	//b的虚部

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
float qua_dot(const qua& a, const qua& b) {		//四元数a和b的点乘
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}
inline
float qua_length(const qua& a) {		//四元数a的模长
	return sqrt(qua_dot(a, a));
}
inline
qua qua_normalize(const qua& a) {		//单位化四元数
	return a / qua_length(a);
}
inline
qua qua_conjugation(const qua& a)		//返回与a共轭的四元数
{
	return qua(im(-a.x, -a.y, -a.z), a.w);
}
inline
qua qua_inverse(const qua& a)			//返回四元数a的逆
{
	return qua_conjugation(a) / qua_length(a);
}
mat4 qua_cast_to_rotation_mat(const qua& a)	//将一个单位四元数( ||a|| = 1 )转换为旋转矩阵
{
#ifdef DEBUG	//检查是不是单位四元数
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

qua euler_angle_to_qua(float pitch, float yaw, float roll)	//欧拉角表示的旋转转为四元数
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