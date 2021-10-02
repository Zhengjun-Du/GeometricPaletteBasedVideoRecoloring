#pragma once

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include "RgbtSkewPolytope.h"
#include "utility.h"
#include "Frame.h"
#include "FrameBlock.h"

#define REFINE_RGB	1
#define REFINE_T	2

using namespace std; 
using namespace cv;

class VideoPalette
{
public:
	//all video frames
	vector<Frame> m_frames;

	//rgbt convex hull built between adjacent frames’ convex hulls
	vector<RgbtConvexhull> m_rgbt_cvxhulls;

	//block info
	FrameBlock m_block_merge;

	//the resulting skew polytope
	RgbtSkewPolytope m_polytope_palette;

	// current refining vertex id
	int m_curr_refine_vidx;

	int m_refine_rgb_or_t;

public:
	VideoPalette() {};

	//read each frame's rgb and convex hull
	void ReadVideoAndFrameRgbConvexhulls(string base_dir);

	//build the rgbt convex hull on adjacent frames' convex hulls
	void BuildRgbtConvexHullOnAdjFrames();

	//blodk merging
	void BuildAndMergeFrameBlocks(int dir_id = 0, int file_id = 0);

	//vertex removing
	void SimplifyVideoPalette();

	//video palette vertex refining
	void RefineVideoPalette();
	double RefineSingleVertex(int vid);

	//energy function
	double GetEnergy(double& rloss, double& closs, double& sloss);

	//for simple parallel
	double GetEnergy_parallel(RgbtSkewPolytope& RSP, double& rloss, double& closs, double& sloss);

	//output the resulting skew polytope and its slices
	void WriteEachFramePalette(string sliced_3d_poly_path, string sliced_polyface_path);

	void GetVideoAllFrameLoss(double& rloss, double& closs, double& sloss);
	void PostProcess();
};

//video palette vertex refinement function
double global_ObjectFunction(unsigned n, const double *x, double *grad, void *data);