#pragma once
#include <opencv2/core.hpp>
#include <vector>
#include <time.h>
#include <set>
#include <map>
#include "SystemParameter.h"
using namespace cv;
using namespace std;

#define ESP (0.00000001)

struct Edge4d {
	cv::Vec4d v1, v2;
	int v1_id, v2_id;
	double len, rgb_len, cost, slope;

	//used for vertex removing
	//one edge may cross multiple blocks, whitin each block, the cross edge may corresponding different topologies in different blocks
	map<int, int> blk_vertid_maps;

	Edge4d() {}

	Edge4d(cv::Vec4d& v1_, cv::Vec4d& v2_) {
		v1 = v1_;
		v2 = v2_;
		cv::Vec4d e(v1 - v2);
		rgb_len = cv::norm(cv::Vec3d(e[0], e[1], e[2]));
		len = cv::norm(v1 - v2);
	}

	double GetSlope() {
		cv::Vec4d e(v1 - v2);
		rgb_len = cv::norm(cv::Vec3d(e[0], e[1], e[2]));
		slope = rgb_len / fabs(v1[3] - v2[3]);
		return slope;
	}

	Edge4d(cv::Vec4d& v1_, cv::Vec4d& v2_, int& id1, int& id2) {
		v1 = v1_;
		v2 = v2_;
		v1_id = id1;
		v2_id = id2;
		len = cv::norm(v1 - v2);

		cv::Vec4d e(v1 - v2);
		rgb_len = cv::norm(cv::Vec3d(e[0], e[1], e[2]));
		slope = rgb_len / fabs(v1[3] - v2[3]);
	}

	void update(cv::Vec4d& v1_, cv::Vec4d& v2_, int& id1, int& id2) {
		v1 = v1_;
		v2 = v2_;
		v1_id = id1;
		v2_id = id2;
		len = cv::norm(v1 - v2);

		cv::Vec4d e(v1 - v2);
		rgb_len = cv::norm(cv::Vec3d(e[0], e[1], e[2]));
		slope = rgb_len / fabs(v1[3] - v2[3]);
	}

	Edge4d(cv::Vec4d& v1_, cv::Vec4d& v2_, int& id1, int& id2, map<int, int> blk_vertid_pairs) {
		v1 = v1_;
		v2 = v2_;
		v1_id = id1;
		v2_id = id2;
		len = cv::norm(v1 - v2);
		blk_vertid_maps = blk_vertid_pairs;

		cv::Vec4d e(v1 - v2);
		rgb_len = cv::norm(cv::Vec3d(e[0], e[1], e[2]));
		slope = rgb_len / fabs(v1[3] - v2[3]);
	}
	/*
	bool operator < (const Edge4d& e)const {
	return len < e.len;
	}*/

	bool operator < (const Edge4d& e)const {
		cv::Vec3d a(v1[0] - v2[0], v1[1] - v2[1], v1[2] - v2[2]);
		cv::Vec3d b(e.v1[0] - e.v2[0], e.v1[1] - e.v2[1], e.v1[2] - e.v2[2]);
		return cv::norm(a) < cv::norm(b);
	}
};
bool cmp_rgbt_edge(Edge4d& e1, Edge4d& e2);
bool cmp_rgb_edge(Edge4d& e1, Edge4d& e2);

struct vec3d_comp {
	bool operator()(const cv::Vec3d& v1, const cv::Vec3d& v2) const {
		if (fabs(v1[0] - v2[0]) > ESP)
			return v1[0] < v2[0];
		else if (fabs(v1[1] - v2[1]) > ESP)
			return v1[1] < v2[1];
		else if (fabs(v1[2] - v2[2]) > ESP)
			return v1[2] < v2[2];
		return false;
	}
};

struct Edge3d {
	cv::Vec3d v1, v2;
	int v1_id, v2_id;
	double len;
	Edge3d() {}
	Edge3d(cv::Vec3d& v1_, cv::Vec3d& v2_, int id1, int id2) {
		v1 = v1_;
		v2 = v2_;
		v1_id = id1;
		v2_id = id2;
		len = cv::norm(v1 - v2);
	}

	Edge3d(cv::Vec3d& v1_, cv::Vec3d& v2_) {
		v1 = v1_;
		v2 = v2_;
		len = cv::norm(v1 - v2);
	}

	bool operator < (const Edge3d& e)const {
		return len < e.len;
	}
};

/*
fslice: t A x B C y D E z F: express the face of a 3D slice in a 4D rgbt polytope at time t
where A B C D E F are 4D skew polytope's vertex ids，
P,Q,R are the intersections of super plane t and the skew polytope

P = v[a_idx] * (1 - w_ab) + v[b_idx] * w_ab
Q = v[c_idx] * (1 - w_cd) + v[d_idx] * w_cd
R = v[e_idx] * (1 - w_ef) + v[f_idx] * w_ef

P、Q、R are orderly，follow the right-hand spiral rule, with the normal pointing outward
*/

struct SlicedFace {
	int fid;
	int a_idx, b_idx;
	int c_idx, d_idx;
	int e_idx, f_idx;
	double w_ab, w_cd, w_ef;
};

struct BlockMergeLoss {
	double loss, rloss, closs;
	int src_blk_id, tar_blk_id;
	BlockMergeLoss() {}
	BlockMergeLoss(double loss_, double rloss_, double closs_, int src_ = 0, int tar_ = 0) {
		loss = loss_;
		rloss = rloss_;
		closs = closs_;
		src_blk_id = src_;
		tar_blk_id = tar_;
	}
	bool operator <(BlockMergeLoss& bml) {
		return loss < bml.loss;
	}
};

int max4(int v1, int v2, int v3, int v4);
int min4(int v1, int v2, int v3, int v4);
bool rayThroughTriangle(const cv::Vec3d& origin, cv::Vec3d& rayDir, cv::Vec3d tri[3], cv::Vec3d& interPoint);
string edgeVec2String(vector<Edge3d> edges);
string edgeVec2String(vector<Edge4d> edges);
string block2String(vector<int>& block);
string frameVert2String(int fid, cv::Vec3d& v);

double DisOfMidVert2OppsiteEdge(cv::Vec4d& u, cv::Vec4d& v, cv::Vec4d& w);

cv::Vec3d GetAveragePosition(vector<cv::Vec3d>& vertices);
bool OrientationOfTriangle(cv::Vec3d& v1, cv::Vec3d& v2, cv::Vec3d& v3);
