//
// Created by YILI WANG on 3/11/19.
//

#ifndef PALEfloatfloatE_EDIfloat_VEC3_H
#define PALEfloatfloatE_EDIfloat_VEC3_H

#include "utility.h"

#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <float.h>

class vec3
{
public:

	vec3(float x_, float y_, float z_) { v[0] = x_; v[1] = y_; v[2] = z_; }

        vec3(const vec3  & p)
	{
		v[0] = (float)(p[0]);
		v[1] = (float)(p[1]);
		v[2] = (float)(p[2]);
	}

	vec3() { v[0] = 0; v[1] = 0; v[2] = 0; }

	inline  float x() const { return v[0]; }
	inline  float y() const { return v[1]; }
	inline  float z() const { return v[2]; }

	inline  float operator [] (unsigned int c) const
	{
		return v[c];
	}
	inline  float & operator [] (unsigned int c)
	{
		return v[c];
	}

	static vec3 Zero() { return vec3(0, 0, 0); }

	void setZero()
	{
		v[0] = 0;
		v[1] = 0;
		v[2] = 0;
	}

        void operator = (const vec3 & other)
        {
                v[0] = other.x();
                v[1] = other.y();
                v[2] = other.z();
        }

        void operator += (const vec3 & other)
        {
                v[0] += other.x();
                v[1] += other.y();
                v[2] += other.z();
        }

	void operator -= (const vec3 & other)
	{
		v[0] -= other.x();
		v[1] -= other.y();
		v[2] -= other.z();
	}

	// floathis is going to create problems if the compiler needs to resolve umbiguous casts, but it's the cleaner way to do it
	void operator *= (int s)
	{
		v[0] *= s;
		v[1] *= s;
		v[2] *= s;
	}
	void operator *= (unsigned int s)
	{
		v[0] *= s;
		v[1] *= s;
		v[2] *= s;
	}
	void operator *= (float s)
	{
		v[0] *= s;
		v[1] *= s;
		v[2] *= s;
	}
	void operator *= (double s)
	{
		v[0] *= s;
		v[1] *= s;
		v[2] *= s;
	}
	void operator /= (int s)
	{
		v[0] /= s;
		v[1] /= s;
		v[2] /= s;
	}
	void operator /= (unsigned int s)
	{
		v[0] /= s;
		v[1] /= s;
		v[2] /= s;
	}
	void operator /= (float s)
	{
		v[0] /= s;
		v[1] /= s;
		v[2] /= s;
	}
	void operator /= (double s)
	{
		v[0] /= s;
		v[1] /= s;
		v[2] /= s;
	}

	vec3 getOrthogonal() const
	{
		if (v[0] == 0)
		{
			return vec3(0, v[2], -v[1]);
		}
		else if (v[1] == 0)
		{
			return vec3(v[2], 0, -v[0]);
		}

		return vec3(v[1], -v[0], 0);
	}


	float sqrnorm() const
	{
		return v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	}

	float norm() const
    {
        return mysqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	}

	void normalize()
	{
		float _n = norm();
		v[0] /= _n;
		v[1] /= _n;
		v[2] /= _n;
	}

	//float dot(const vec3& anotherVec) const {
	//	return ::dot(*this,anotherVec);
	//}
        operator QString() const {
            QString result;
            QTextStream(&result) << "vec3(" << v[0] << ", " << v[1] << ", " << v[2] << ")";
            return result;
        }

private:
	float v[3];
};




inline
float dot(const vec3 & p1, const vec3 & p2)
{
	return p1.x() * p2.x() + p1.y() * p2.y() + p1.z() * p2.z();
}
inline
vec3 cross(const vec3 & p1, const vec3 & p2) {
	return vec3(
		p1.y() * p2.z() - p1.z() * p2.y(),
		p1.z() * p2.x() - p1.x() * p2.z(),
		p1.x() * p2.y() - p1.y() * p2.x()
	);
}

inline
vec3 operator + (const vec3 & p1, const vec3 & p2)
{
	return vec3(p1.x() + p2.x(), p1.y() + p2.y(), p1.z() + p2.z());
}
inline
vec3 operator - (const vec3 & p1, const vec3 & p2)
{
	return vec3(p1.x() - p2.x(), p1.y() - p2.y(), p1.z() - p2.z());
}


inline
vec3 operator - (const vec3 & p2)
{
	return vec3(-p2.x(), -p2.y(), -p2.z());
}

inline
vec3 operator * (const vec3 & p, int s)
{
	return vec3(s*p.x(), s*p.y(), s*p.z());
}
inline
vec3 operator * (const vec3 & p, float s)
{
	return vec3(s*p.x(), s*p.y(), s*p.z());
}
inline
vec3 operator * (const vec3 & p, double s)
{
	return vec3(s*p.x(), s*p.y(), s*p.z());
}
inline
vec3 operator * (int s, const vec3 & p)
{
	return vec3(s*p.x(), s*p.y(), s*p.z());
}
inline
vec3 operator * (float s, const vec3 & p)
{
	return vec3(s*p.x(), s*p.y(), s*p.z());
}
inline
vec3 operator * (double s, const vec3 & p)
{
	return vec3(s*p.x(), s*p.y(), s*p.z());
}


inline
vec3 operator / (const vec3 & p, int s)
{
	return vec3(p.x() / s, p.y() / s, p.z() / s);
}
inline
vec3 operator / (const vec3 & p, float s)
{
	return vec3(p.x() / s, p.y() / s, p.z() / s);
}
inline
vec3 operator / (const vec3 & p, double s)
{
	return vec3(p.x() / s, p.y() / s, p.z() / s);
}


inline
float operator * (const vec3 & p1, const vec3 & p2)
{
	return p1.x() * p2.x() + p1.y() * p2.y() + p1.z() * p2.z();
}


inline
vec3 operator % (const vec3 & p1, const vec3 & p2)
{
	return cross(p1, p2);
}


inline std::ostream & operator << (std::ostream & s, vec3 const & p)
{
	s << p[0] << " \t" << p[1] << " \t" << p[2];
	return s;
}


#endif //PALEfloatfloatE_EDIfloat_VEC3_H
