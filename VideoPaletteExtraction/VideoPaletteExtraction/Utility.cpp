#include "utility.h"
#include <math.h>
#include <algorithm>
#include <iostream>
using namespace std;

bool cmp_rgbt_edge(Edge4d& e1, Edge4d& e2) {
	return e1.cost > e2.cost;
}

bool cmp_rgb_edge(Edge4d& e1, Edge4d& e2) {
	return e1.rgb_len > e2.rgb_len;
}

int max4(int v1, int v2, int v3, int v4) {
	int res = max(max(v1, v2), max(v3, v4)); 
	return res;
}

int min4(int v1, int v2, int v3, int v4) {
	int res = min(min(v1, v2), min(v3, v4));
	return res;
}

// get the intersection of a ray and a triangle(v0，v1，v2)
// reference:https://blog.csdn.net/linolzhang/article/details/55548707
bool rayThroughTriangle(const cv::Vec3d& origin, cv::Vec3d& rayDir, cv::Vec3d tri[3], cv::Vec3d& interPoint)
{
	// the ray is parallel to triangle? 
	cv::Vec3d v0 = tri[0], v1 = tri[1], v2 = tri[2];
	cv::Vec3d u = v1 - v0;	// edge1
	cv::Vec3d v = v2 - v0;	// edge2
	cv::Vec3d normal = u.cross(v);	// normal

	if (normal == cv::Vec3d(0.0f, 0.0f, 0.0f))  // triangle is degenerate 
		return false;

	//cout << u << "\t" << v << "\t" << normal << endl;

	// the angle of the ray and the plane
	double b = normal.dot(rayDir);
	if (fabs(b) < 1e-5)
		return false;

	// (v0 -> ray start vertex)
	cv::Vec3d w0 = origin - v0;
	double a = -(normal.dot(w0));

	// get intersect point of ray with triangle plane   
	float r = a / b;
	if (r < 0.0f)                 // ray goes away from triangle   
		return false;             // => no intersect   
								  // for a segment, also test if (r > 1.0) => no intersect   
	// the intersection of the ray and the plane
	interPoint = origin + r * rayDir;

	// intersection is inside the triangle?  
	double uu = u.dot(u);
	double uv = u.dot(v);
	double vv = v.dot(v);
	cv::Vec3d w = interPoint - v0;
	double wu = w.dot(u);
	double wv = w.dot(v);
	double D = uv * uv - uu * vv;

	// get and test parametric coords   
	double s = (uv * wv - vv * wu) / D;
	if (s < 0.0f || s > 1.0f)       // interPoint is outside the triangle
		return false;

	double t = (uv * wu - uu * wv) / D;
	if (t < 0.0f || (s + t) > 1.0f) // interPoint is outside the triangle
		return false;

	return true;
}

string edgeVec2String(vector<Edge3d> edges) {
	//sort(edges.begin(), edges.end());
	string result;
	for (int i = 0; i < edges.size(); i++) {
		result += to_string(edges[i].v1_id) + "-" + to_string(edges[i].v2_id) + "-";
	}
	return result;
}

string edgeVec2String(vector<Edge4d> edges) {
	//sort(edges.begin(), edges.end());
	string result;
	for (int i = 0; i < edges.size(); i++) {
		//result += "(" + to_string(edges[i].v1_id + 1) + "-" + to_string(edges[i].v2_id + 1) + ")-";
		result += "(" + to_string(edges[i].v1_id ) + "-" + to_string(edges[i].v2_id) + ")-";

	}
	return result;
}

string block2String(vector<int>& block) {
	string result = "BF-" + to_string(block.front()) + "-" + to_string(block.back());
	return result;
}

string frameVert2String(int fid, cv::Vec3d& v) {
	string result = to_string(fid) + "-" + to_string(v[0]) + to_string(v[1]) + to_string(v[2]);
	return result;
}


cv::Vec3d GetAveragePosition(vector<cv::Vec3d>& vertices) {
	cv::Vec3d res(0, 0, 0);
	for (int i = 0; i < vertices.size(); i++) {
		res[0] += vertices[i][0];
		res[1] += vertices[i][1];
		res[2] += vertices[i][2];
	}
	res[0] /= vertices.size();
	res[1] /= vertices.size();
	res[2] /= vertices.size();
	return res;
}

bool OrientationOfTriangle(cv::Vec3d& v1, cv::Vec3d& v2, cv::Vec3d& v3) {
	cv::Vec3d v12 = v2 - v1;
	cv::Vec3d v23 = v3 - v2;
	double ans = (v12[1] * v23[2] - v12[2] * v23[1]) - (v12[0] * v23[2] - v12[2] * v23[0]) + (v12[0] * v23[2] - v12[1] * v23[0]);
	return ans > 0;
}

double DisOfMidVert2OppsiteEdge(cv::Vec4d& u, cv::Vec4d& v, cv::Vec4d& w) {
	cv::Vec4d uv = v - u;
	cv::Vec4d uw_normal = cv::normalize(w - u);
	cv::Vec4d proj = uv.dot(uw_normal) * uw_normal;
	double h = cv::norm(uv - proj);
	return h;
} 