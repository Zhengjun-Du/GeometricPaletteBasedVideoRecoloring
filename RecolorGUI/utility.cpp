#include "utility.h"
#include <cmath>
#include <QDebug>

// utility usually used
//   4d poly cutting
//   hsv rgb converting
//   vector operations

bool cut(double time, vec4 p,  vec4 q, double &r, double &g, double &b)
{
    if(p[3] > q[3]) { vec4 tmp = p; p = q; q = tmp; }

    if(p[3] < time + 1e-6 && time < q[3] + 1e-6)
    {
        double wp = q[3] - time;
        double wq = time - p[3];

        r = (p[0] * wp + q[0] * wq) / (wp + wq);
        g = (p[1] * wp + q[1] * wq) / (wp + wq);
        b = (p[2] * wp + q[2] * wq) / (wp + wq);
        return true;
    }
    else
    {
        return false;
    }
}


void RGBtoHSV(double src_r, double src_g, double src_b,
                    double &dst_h, double &dst_s, double &dst_v)
{
    float r = src_r;
    float g = src_g;
    float b = src_b;

    float h, s, v; // h:0-360.0, s:0.0-1.0, v:0.0-1.0

    float max = 0; // std::max(std::max(r, g), b);
    float min = 0; // std::max(std::max(r, g), b);

    if(r > max) max = r;
    if(g > max) max = g;
    if(b > max) max = b;
    if(r < min) min = r;
    if(g < min) min = g;
    if(b < min) min = b;

    v = max;

    if (max == 0.0f) {
        s = 0;
        h = 0;
    }
    else if (max - min == 0.0f) {
        s = 0;
        h = 0;
    }
    else {
        s = (max - min) / max;

        if (max == r) {
            h = 60 * ((g - b) / (max - min)) + 0;
        }
        else if (max == g) {
            h = 60 * ((b - r) / (max - min)) + 120;
        }
        else {
            h = 60 * ((r - g) / (max - min)) + 240;
        }
    }

    if (h < 0) h += 360.0f;

    dst_h = (h); // dst_h : 0-360
    dst_s = (s); // dst_s : 0-255
    dst_v = (v); // dst_v : 0-255
}

void cross(double x1, double y1, double z1, double x2, double y2, double z2, double &x, double &y, double &z)
{
    x = y1 * z2 - z1 * y2;
    y = z1 * x2 - x1 * z2;
    z = x1 * y2 - y1 * x2;
}

double norm2(double x, double y, double z)
{
    return x * x + y * y + z * z;
}

double angle(double x1, double y1, double z1, double x2, double y2, double z2)
{
    double norm2_1 = norm2(x1, y1, z1);
    double norm2_2 = norm2(x2, y2, z2);

    if(norm2_1 < 1e-6 || norm2_2 < 1e-6) return 0;

    double c = (x1 * x2 + y1 * y2 + z1 * z2) / mysqrt(norm2_1 * norm2_2);
    if(c > 1) c = 1;
    if(c < -1) c = -1;
    return myacos(c);

}

double dot(double x1, double y1, double z1, double x2, double y2, double z2)
{
    return x1 * x2 + y1 * y2 + z1 * z2;
}

myfloat3::myfloat3(float _x, float _y, float _z) : x(_x), y(_y), z(_z)
{
}

myfloat3::myfloat3() : x(0.f), y(0.f), z(0.f)
{
}

const float myfloat3::norm2() const
{
    return x * x + y * y + z * z;
}

const float myfloat3::norm() const
{
    float ans = x * x + y * y + z * z;
    if(ans < 0) ans = 0;
    return sqrtf(ans);
}

float myfloat3::dot(myfloat3 another)
{
    return x * another.x + y * another.y + z * another.z;
}

myfloat3 myfloat3::cross(myfloat3 another)
{
    const float &x_ = another.x;
    const float &y_ = another.y;
    const float &z_ = another.z;

    return myfloat3(
                y * z_ - z * y_,
                z * x_ - x * z_,
                x * y_ - y * x_
                );
}

const myfloat3 myfloat3::operator +(const myfloat3 another) const
{
    const float &x_ = another.x;
    const float &y_ = another.y;
    const float &z_ = another.z;

    return myfloat3(x + x_, y + y_, z + z_);
}

const myfloat3 myfloat3::operator -(const myfloat3 another) const
{
    const float &x_ = another.x;
    const float &y_ = another.y;
    const float &z_ = another.z;

    return myfloat3(x - x_, y - y_, z - z_);
}

const myfloat3 myfloat3::operator *(float k) const
{
    return myfloat3(x * k, y * k, z * k);
}

const myfloat3 myfloat3::operator /(float k) const
{
    k = 1.f / k;
    return myfloat3(x * k, y * k, z * k);
}

float myfloat3::getX() const
{
    return x;
}

float myfloat3::getY() const
{
    return y;
}

float myfloat3::getZ() const
{
    return z;
}

float dot(myfloat3 x, myfloat3 y)
{
    return x.dot(y);
}

myfloat3 cross(myfloat3 x, myfloat3 y)
{
    return x.cross(y);
}

QDebug operator<<(QDebug debug, const myfloat3 &c)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << '(' << c.getX() << ", " << c.getY() << ", " << c.getZ() << ')';

    return debug;
}


//Yili Wang's code
vec3 closesPointOnTriangle(const vec3 *triangle, const vec3 &sourcePosition)
{
	vec3 edge0 = triangle[1] - triangle[0];
	vec3 edge1 = triangle[2] - triangle[0];
	vec3 v0 = triangle[0] - sourcePosition;

	float a = dot(edge0, edge0);
	float b = dot(edge0, edge1);
	float c = dot(edge1, edge1);
	float d = dot(edge0, v0);
	float e = dot(edge1, v0);

	float det = a*c - b*b;
	float s = b*e - c*d;
	float t = b*d - a*e;

	if (s + t < det)
	{
		if (s < 0.f)
		{
			if (t < 0.f)
			{
				if (d < 0.f)
				{
					s = my_util::clamp(-d / a, 0.f, 1.f);
					t = 0.f;
				}
				else
				{
					s = 0.f;
					t = my_util::clamp(-e / c, 0.f, 1.f);
				}
			}
			else
			{
				s = 0.f;
				t = my_util::clamp(-e / c, 0.f, 1.f);
			}
		}
		else if (t < 0.f)
		{
			s = my_util::clamp(-d / a, 0.f, 1.f);
			t = 0.f;
		}
		else
		{
			float invDet = 1.f / det;
			s *= invDet;
			t *= invDet;
		}
	}
	else
	{
		if (s < 0.f)
		{
			float tmp0 = b + d;
			float tmp1 = c + e;
			if (tmp1 > tmp0)
			{
				float numer = tmp1 - tmp0;
				float denom = a - 2 * b + c;
				s = my_util::clamp(numer / denom, 0.f, 1.f);
				t = 1 - s;
			}
			else
			{
				t = my_util::clamp(-e / c, 0.f, 1.f);
				s = 0.f;
			}
		}
		else if (t < 0.f)
		{
			if (a + d > b + e)
			{
				float numer = c + e - b - d;
				float denom = a - 2 * b + c;
				s = my_util::clamp(numer / denom, 0.f, 1.f);
				t = 1 - s;
			}
			else
			{
				s = my_util::clamp(-e / c, 0.f, 1.f);
				t = 0.f;
			}
		}
		else
		{
			float numer = c + e - b - d;
			float denom = a - 2 * b + c;
			s = my_util::clamp(numer / denom, 0.f, 1.f);
			t = 1.f - s;
		}
	}

    return triangle[0] + s * edge0 + t * edge1;
}

float myasin(float x)
{
    if(x > 1) x = 1;
    if(x < -1) x = -1;
    return asin(x);
}

float myacos(float x)
{
    if(x > 1) x = 1;
    if(x < -1) x = -1;
    return acos(x);
}

float mysqrt(float x)
{
    if(x < 0) x = 0;
    return sqrt(x);
}

float mylog(float x)
{
    if(x < 0) x = 0;
    return log(x);
}
