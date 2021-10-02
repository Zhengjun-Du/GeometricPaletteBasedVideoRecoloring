#pragma once

#include <vector>
#include <opencv2/opencv.hpp>
#include "RgbConvexhull.h"
using namespace std;

class Frame
{
public: 
	//rgb data
	vector<cv::Vec3d> m_img;

	//convex hull
	RgbConvexhull m_cvxhull;

	//kd-tree of the sample pixels
	cv::flann::Index* m_kdtree;
	cv::Mat m_pixel_mat;

	//index of sample pixel
	vector<int> m_samp_pix_ids;

	//reconstrction loss and compactness loss
	double m_rloss, m_closs;

	//frame id
	int m_frm_id;

	//the block current frame locate in
	int m_blk_id;

	//normalize of time
	double m_normal_t;

	//vertex refiment radius
	double m_refine_size;
	bool m_is_keyframe;
	int m_curr_refine_vidx;

public:
	Frame();
	Frame(string img_path, string ch_path, int fid, bool is_keyframe, double normal_t);
	void ReadFrame(string img_path);
	void ReadConvexHull(string ch_path);
	void ReBuildConvexHull();
	void BuildKdTree();
	double GetReconstuctLoss();
	double GetCompactLoss();
	double GetLoss();
	void Refine();
	bool RefineSingleVertex(int vid, double range);
	void WriteFrame(string path);

	//for debug
	vector<cv::Vec3d> GetInsidePixels();
};

double gloabl_FrmOptimal(unsigned n, const double *x, double *grad, void *data);
double constraint(unsigned n, const double *x, double *grad, void *data);