#pragma once

#include "Frame.h"
#include "RgbtConvexhull.h"
#include "RgbtSkewPolytope.h"

#define LEFT	0
#define RIGHT	1

class FrameBlock 
{
public:
	//all frames
	vector<Frame>  m_frames;  

	//adjacent frames' RGBT convex hull
	vector<RgbtConvexhull> m_rgbt_cvxhulls;

	//frame ids in each block
	vector<vector<int> > m_frm_blocks;

	//the entire skew polytope
	RgbtSkewPolytope m_skew_polytope;

	//loss of each block
	map<string, cv::Vec3d> m_block_rcloss_map;

	//string:"a-b",vector<Frame a~b>:
	map<string, vector<Frame> > m_block_updated_frames_map;

public:
	FrameBlock() {}
	FrameBlock(vector<Frame>& frames, vector<RgbtConvexhull>& adj_rgbt_chs);

	//initially divide all video frames into several blocks
	void BuildInitialFrameBlocks();

	//block merge
	void MergeFrameBlocks();

	//merge 2 similar adjacent blocks
	double MergeAdjacentBlocks(int a, int b, double& rloss, double& closs);

	//calculate reconstruct loss and compact loss
	double ComputeOverallReconstructAndCompactLoss();

	//calculate single block's reconstruct loss and compact loss
	cv::Vec3d ComputeSingleBlockReconstructAndCompactLoss(vector<Frame>& block_frms);

	//update topology after block mergeing
	void UpdateSkewPolyhedronTopologyWithin(int beg_fid, int end_fid);

	//generate skew polytope
	void GenerateSkewPolyhedron_fine();
	void GenerateSkewPolyhedron_coarse();

	//for debug
	void ComputeAveFrameLoss(double& rloss, double& closs);
};