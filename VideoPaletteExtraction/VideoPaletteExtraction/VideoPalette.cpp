#include "VideoPalette.h"
#include <fstream>
#include <algorithm>
#include <omp.h>
#include "nlopt.h"
using namespace std;

void VideoPalette::ReadVideoAndFrameRgbConvexhulls(string base_dir) {

	string img_path = base_dir + "%05d.png";
	string cvx_path = base_dir + "rgb_cvx_%05d.obj";

	//1.sampling step
	int frame_step = SysParameter::FrameCnt() / SysParameter::SampFrameCnt();

	//2.read frames and convex hulls
	m_frames.clear();
	m_frames.resize(SysParameter::FrameCnt());

	#pragma omp parallel for num_threads(32)
	for (int i = 0; i < SysParameter::FrameCnt(); i++) {
		std::cout << "Read Frame " << i << endl << endl;

		//2.1 read frame and convex hull
		string img_file = format(img_path.c_str(), i);
		string ch_file = format(cvx_path.c_str(), i);
		bool is_keyframe = !(i % frame_step);
		m_frames[i] = Frame(img_file, ch_file, i, is_keyframe, (i*1.0) / (SysParameter::FrameCnt() - 1));

		//2.2 refine convex hull
		m_frames[i].Refine();
		m_frames[i].ReBuildConvexHull();
		m_frames[i].m_cvxhull.CorrectNormal();

		//2.3 output refined convex hull
		string ref_cvx_path = base_dir + "/rgb_cvxf_%05d.obj";
		ref_cvx_path = format(ref_cvx_path.c_str(), i);
		m_frames[i].WriteFrame(ref_cvx_path);
	}

	//3. build the adjacent frames' rgbt convex hull
	BuildRgbtConvexHullOnAdjFrames();
}

//build the adjacent frames' rgbt convex hull
void VideoPalette::BuildRgbtConvexHullOnAdjFrames() {
	m_rgbt_cvxhulls.clear();
	for (int i = 0; i < SysParameter::FrameCnt() - 1; i++) {
		RgbtConvexhull rgbt_ch(m_frames[i], m_frames[i + 1]);
		m_rgbt_cvxhulls.push_back(rgbt_ch);
	}

	//simplify the rgbt convex hull, reduce the redundant edges
	cout << "Remove reduandant rgbt convex hull redundant cross edges..." << endl;

	#pragma omp parallel for num_threads(32)
	for (int i = 0; i < m_rgbt_cvxhulls.size(); i++) {
		RgbConvexhull ch1 = m_frames[i].m_cvxhull;
		RgbConvexhull ch2 = m_frames[i + 1].m_cvxhull;

		cout << "process rgbt convex hull: " << i << endl;

		ReMoveStatus remove_stat = m_rgbt_cvxhulls[i].RemoveRedundantCrossEdges(ch1, ch2);

		//exception, relax it
		if (FinalRemoveEdgeSuccess != remove_stat) {
			m_frames[i + 1].m_cvxhull = m_frames[i].m_cvxhull;
			m_rgbt_cvxhulls[i] = RgbtConvexhull(m_frames[i], m_frames[i + 1]);

			//when modify frame i+1's topology，the rgbt convex hull of frame i+1 and frame i+2 should be changed accordingly
			if(i + 2 < m_rgbt_cvxhulls.size())
				m_rgbt_cvxhulls[i+1] = RgbtConvexhull(m_frames[i+1], m_frames[i + 2]);

			ch1 = m_frames[i].m_cvxhull;
			ch2 = m_frames[i + 1].m_cvxhull;
			m_rgbt_cvxhulls[i].RemoveRedundantCrossEdges(ch1, ch2);
		}
	}
}

//block merging
void VideoPalette::BuildAndMergeFrameBlocks(int dir_id, int file_id) {

	//1. group adjacent frames with consistent convex hull topology into a block
	m_block_merge = FrameBlock(m_frames, m_rgbt_cvxhulls);
	m_block_merge.BuildInitialFrameBlocks();

	//2. merge similar adjacent blocks
	m_block_merge.MergeFrameBlocks();

	//3. update frames' topologies
	m_frames = m_block_merge.m_frames;

	//4. update polytope
	m_polytope_palette = m_block_merge.m_skew_polytope;
	m_polytope_palette.m_pframes = &m_frames;
	m_polytope_palette.m_frm_blocks = m_block_merge.m_frm_blocks;

	m_rgbt_cvxhulls = m_block_merge.m_rgbt_cvxhulls;
}

//polytope(video palette) vertex removal
void VideoPalette::SimplifyVideoPalette() {

	int initVertNum = m_polytope_palette.m_vertices.size(); 

	//1. build the vertex priority queue(only consider one-to-one simple edges)
	//triangle(a,b,c): distance of b to edge(a,c), if a,b,c lie in a line, the distance = 0 with highest priority
	m_polytope_palette.BuildVertexProqueue();

	double rloss = 0, closs = 0, sloss = 0;
	double init_loss = GetEnergy_parallel(m_polytope_palette, rloss, closs, sloss);

	//2. remove redundant vertex
	int s = 1;
	while (!m_polytope_palette.m_spv_proque.empty()) {
		clock_t start = clock();

		//2.1 select candidate vertices
		vector<PolyVertex4D> candiate_verts;
		for (int i = 0; i < min(10, (int)m_polytope_palette.m_spv_proque.size()); i++) {
			PolyVertex4D curr_sv = m_polytope_palette.m_spv_proque.top();
			candiate_verts.push_back(curr_sv);
			m_polytope_palette.m_spv_proque.pop();
		}

		//2.2 check the candidates
		int best_v = 0;
		double best_loss = DBL_MAX, best_rloss = 0, best_closs = 0, best_sloss = 0;
		bool have_fund_candidates = false;

		#pragma omp parallel for num_threads(32)
		for (int i = 0; i < candiate_verts.size(); i++) {

			RgbtSkewPolytope BakPolyhedronPalette = m_polytope_palette;

			PolyVertex4D curr_sv = candiate_verts[i];
			int curr_vid = curr_sv.vid;

			//a vertex is not allowed to remove if both its left and right vertex degree > 2, ambiguity may arise.
			set<int>::iterator it = BakPolyhedronPalette.m_vert_adjverts[curr_vid].begin();
			int LR1 = *(it), LR1_deg = BakPolyhedronPalette.m_vert_adjverts[LR1].size();
			int LR2 = *(++it), LR2_deg = BakPolyhedronPalette.m_vert_adjverts[LR2].size();
			if (LR1_deg > 2 && LR2_deg > 2) //&& min_dis > 0.1)
				continue;

			//calculate the loss when remove current vertex
			BakPolyhedronPalette.TryRemoveVertex(curr_vid);
			double r_loss = 0, c_loss = 0, s_loss = 0;
			double loss = GetEnergy_parallel(BakPolyhedronPalette, r_loss, c_loss, s_loss);

			//std::cout << "merge step: " << s <<", curr_id:"<< cuur_vid<< "  loss:" << loss << " rloss: " << r_loss << "," << c_loss << "," << s_loss << endl;

			//update
			if (loss < best_loss) {
				best_loss = loss;
				best_rloss = r_loss;
				best_closs = c_loss;
				best_sloss = s_loss;
				best_v = curr_vid;
				have_fund_candidates = true;
			}
		}

		//2.4 remove vertex
		if (have_fund_candidates) {

			//reset priority queue
			for (int j = 0; j < candiate_verts.size(); j++)
				m_polytope_palette.m_spv_proque.push(candiate_verts[j]);

			if (best_loss < init_loss * SysParameter::SimplifyTh())
				m_polytope_palette.RemoveVertex(best_v);
			else {
				cout << "curr loss exceed much initial loss:" << best_loss << endl;
				break;
			}

			clock_t end = clock();
			std::cout << "merge step: " << s << " remove vertex " << best_v << "  energy:" << best_loss << " loss: " << best_rloss << "," << best_closs << "," << best_sloss << " time: " << end - start << endl << endl;
			s++;
		}
	}
	std::cout << "finish to simplify the skew polytope!" << endl;
}

//vertex refine
void VideoPalette::RefineVideoPalette() {

	set<int> vertices4d_ids;
	vector<Edge4d>& true_edges4d = m_polytope_palette.m_cross_edges;
	for (int i = 0; i < true_edges4d.size(); i++) {
		vertices4d_ids.insert(true_edges4d[i].v1_id);
		vertices4d_ids.insert(true_edges4d[i].v2_id);
	}


	for (int s = 1; s <= 3; s++) {
		std::cout << endl << endl << "start refine vertex in in round " + to_string(s) + "..." << endl;

		//1. refine 2-degree vertices on t-axis
		std::cout << endl << endl << "start refine vertex in T space..." << endl << endl;
		m_refine_rgb_or_t = REFINE_T;
		for (set<int>::iterator it = vertices4d_ids.begin(); it != vertices4d_ids.end(); it++) {

			int vid = *it;
			cout << vid << endl;
			cv::Vec4d vertex = m_polytope_palette.m_vertices[vid];
			double t = vertex[3];
			if (0 < t && t < 1 && m_polytope_palette.m_vert_adjverts[vid].size() == 2)
				RefineSingleVertex(vid);
		}
		std::cout << endl << endl << "end refine vertex in T space" << endl << endl;

		//2. refine 2-degree vertices on rgb space

		std::cout << endl << endl << "start refine vertex in RGB space" + to_string(s) + "..." << endl;
		m_refine_rgb_or_t = REFINE_RGB;
		for (set<int>::iterator it = vertices4d_ids.begin(); it != vertices4d_ids.end(); it++) {
			int vid = *it;
			cv::Vec4d vertex = m_polytope_palette.m_vertices[vid];
			double energy = RefineSingleVertex(vid);
		}
	}
	std::cout << endl << endl << "refine RGB finished!" << endl << endl;
}

//refine single vertex with nlopt
double VideoPalette::RefineSingleVertex(int vid) {
	m_curr_refine_vidx = vid;
	cv::Vec4d vertex = m_polytope_palette.m_vertices[vid];

	double r = vertex[0];
	double g = vertex[1];
	double b = vertex[2];
	double t = vertex[3];

	double t1 = t, t2 = t;
	double tol = 1e-4;
	double s = 0.1;

	if (REFINE_T == m_refine_rgb_or_t) {

		/*
		int fid = t * m_frames.size();
		int block_head_fid = m_polytope_palette.m_frm_blocks[m_frames[fid].m_blk_id].front();
		t1 = m_frames[block_head_fid].m_normal_t;

		int block_tail_fid = m_polytope_palette.m_frm_blocks[m_frames[fid].m_blk_id].back();
		t2 = m_frames[block_tail_fid].m_normal_t;
		*/

		//start time of current block
		int fid = t * m_frames.size(); 
		int block_head_fid = m_polytope_palette.m_frm_blocks[m_frames[fid].m_blk_id].front();
		double block_start_t = m_frames[block_head_fid].m_normal_t;

		//end time of current block
		int block_tail_fid = m_polytope_palette.m_frm_blocks[m_frames[fid].m_blk_id].back();
		double block_end_t = m_frames[block_tail_fid].m_normal_t;

		//time of left neighbor frame
		Edge4d left_edge = m_polytope_palette.m_cross_edges[m_polytope_palette.FindEdgeWithRightVidAs(vid)];
		int left_vid = left_edge.v1_id;
		double left_vert_t = m_polytope_palette.m_vertices[left_vid][3];

		//time of right neighbor frame
		Edge4d right_edge = m_polytope_palette.m_cross_edges[m_polytope_palette.FindEdgeWithLeftVidAs(vid)];
		int right_vid = right_edge.v2_id;
		double right_vert_t = m_polytope_palette.m_vertices[right_vid][3];

		t1 = max(block_start_t, left_vert_t);
		t2 = min(block_end_t, right_vert_t);

		cout <<"optimize range\t" << t1 << "\t" << t2 << endl;

		double diff = t2 - t1;
		t1 += diff * 0.2;
		t2 -= diff * 0.2;
		t = (t1 + t2) / 2.0;

		//only refine t
		s = 0;
	}

	double lb[4] = { max(r - s,0.02),max(g - s,0.02),max(b - s,0.02),t1 };
	double ub[4] = { min(r + s,1.0),min(g + s,1.0),min(b + s,1.0),t2 };

	for (int k = 0; k < 3; k++) {
		if (ub[k] < lb[k])
			ub[k] = lb[k] + 0.01;
	}

	double x[4] = { r,g,b,t };
	double f_min;

	nlopt_opt opter = nlopt_create(NLOPT_LN_COBYLA, 4);
	nlopt_set_lower_bounds(opter, lb);
	nlopt_set_upper_bounds(opter, ub);
	nlopt_set_min_objective(opter, global_ObjectFunction, this);
	nlopt_set_xtol_rel(opter, tol);
	nlopt_set_maxeval(opter, 100);

	std::cout << "before refining: vertex id: " << vid << ",v(" << vertex[0] << "," << vertex[1] << "," << vertex[2] << "," << vertex[3] << ") t:" << t1 << "," << t2 << endl;

	nlopt_result result = nlopt_optimize(opter, x, &f_min);
	if (result) {
		printf("after refining: loss=%g, v =(%g,%g,%g,%g)\n\n", f_min, x[0], x[1], x[2], x[3]);
		m_polytope_palette.m_vertices[vid] = cv::Vec4d(x[0], x[1], x[2], x[3]);
	}

	nlopt_destroy(opter);
	return f_min;
}

//calculate the sampling frames' loss
double VideoPalette::GetEnergy(double& rloss, double& closs, double& sloss) {
	rloss = 0;
	closs = 0;
	sloss = 0;

	#pragma omp parallel for num_threads(32)
	for (int i = 0; i < m_frames.size(); i++) {
		if (m_frames[i].m_is_keyframe) {
			
			//RgbConvexhull rgb_poly = m_polytope_palette.SlicePolyhedronForFrame(i);

			RgbtSkewPolytope RSP = m_polytope_palette;
			RgbConvexhull rgb_poly = RSP.SlicePolyhedronForFrame(i);
			
			/*
			m_frames[i].m_cvxhull = rgb_poly;
			rloss += m_frames[i].GetReconstuctLoss();
			closs += m_frames[i].GetCompactLoss();
			*/

			Frame frame = m_frames[i];
			frame.m_cvxhull = rgb_poly;
			rloss += frame.GetReconstuctLoss();
			closs += frame.GetCompactLoss();
		}
	}

	for (int i = 0; i < m_polytope_palette.m_cross_edges.size(); i++) {
		Edge4d& e = m_polytope_palette.m_cross_edges[i];
		m_polytope_palette.m_cross_edges[i].update(m_polytope_palette.m_vertices[e.v1_id], m_polytope_palette.m_vertices[e.v2_id], e.v1_id, e.v2_id);
		sloss += m_polytope_palette.m_cross_edges[i].slope;
	}

	rloss /= SysParameter::SampFrameCnt(), closs /= SysParameter::SampFrameCnt(), sloss /= m_polytope_palette.m_cross_edges.size();
	double energy = SysParameter::SimplifyRlossWt() * rloss + SysParameter::SimplifyClossWt() * closs + SysParameter::SimplifySlossWt()*sloss;
	return energy;
}

// speed up
double VideoPalette::GetEnergy_parallel(RgbtSkewPolytope& RSP, double& rloss, double& closs, double& sloss) {
	rloss = 0;
	closs = 0;
	sloss = 0;

	int k = 0;
	for (int i = 0; i < m_frames.size(); i++) {
		if (m_frames[i].m_is_keyframe) {
			RgbConvexhull rgb_poly = RSP.SlicePolyhedronForFrame(i);
			Frame frame = m_frames[i];
			frame.m_cvxhull = rgb_poly;

			rloss += frame.GetReconstuctLoss();
			closs += frame.GetCompactLoss();
			k++;
		}
	}

	rloss /= SysParameter::SampFrameCnt(), closs /= SysParameter::SampFrameCnt(), sloss /= RSP.m_cross_edges.size();
	double energy = SysParameter::SimplifyRlossWt() * rloss + SysParameter::SimplifyClossWt() * closs + SysParameter::SimplifySlossWt()*sloss;
	return energy;
}


void VideoPalette::PostProcess() {

	cout << "post process..." << endl;

	set<int> vertices_ids;
	vector<Edge4d>& edges = m_polytope_palette.m_cross_edges;
	for (int i = 0; i < edges.size(); i++) {
		vertices_ids.insert(edges[i].v1_id);
		vertices_ids.insert(edges[i].v2_id);
	}

	for (set<int>::iterator it = vertices_ids.begin(); it != vertices_ids.end(); it++) {
		int vid = *it;
		cv::Vec4d vertex = m_polytope_palette.m_vertices[vid];
		double t = vertex[3];
		if (0 < t && t < 1 && m_polytope_palette.m_vert_adjverts[vid].size() == 2) {

			set<int>::iterator it = m_polytope_palette.m_vert_adjverts[vid].begin();
			int LR1 = *(it), LR1_deg = m_polytope_palette.m_vert_adjverts[LR1].size();
			int LR2 = *(++it), LR2_deg = m_polytope_palette.m_vert_adjverts[LR2].size();

			bool existLoop = false;
			for (int k = 0; k < m_polytope_palette.m_cross_edges.size(); k++) {
				int v1id = m_polytope_palette.m_cross_edges[k].v1_id;
				int v2id = m_polytope_palette.m_cross_edges[k].v2_id;
				if (LR1 == v1id && LR2 == v2id) {
					existLoop = true;
					break;
				}
			}
			if (existLoop)
				continue;

			cv::Vec4d v_LR1 = m_polytope_palette.m_vertices[LR1];
			cv::Vec4d v_LR2 = m_polytope_palette.m_vertices[LR2];

			Edge4d e1(vertex, v_LR1);
			Edge4d e2(vertex, v_LR2);
			double max_dis = max(e1.rgb_len, e2.rgb_len);

			if (max_dis < 0.10) {
				m_polytope_palette.RemoveVertex(vid);
			}
		}
	}
}

void VideoPalette::WriteEachFramePalette(string sliced_3d_poly_path, string sliced_polyface_path) {
	for (int i = 0; i < m_frames.size(); i++)
		m_frames[i].m_cvxhull.CorrectNormal();

	m_polytope_palette.m_pframes = &m_frames;
	m_polytope_palette.SlicePolyhedronForAllFramesAndWrite(sliced_3d_poly_path, sliced_polyface_path);
}

void VideoPalette::GetVideoAllFrameLoss(double& rloss, double& closs, double& sloss) {
	rloss = 0;
	closs = 0;
	sloss = 0;

	for (int i = 0; i < m_frames.size(); i++) {
		RgbConvexhull rgb_poly = m_polytope_palette.SlicePolyhedronForFrame(i);
		Frame f = m_frames[i];
		f.m_cvxhull = rgb_poly;
		rloss += f.GetReconstuctLoss();
		closs += f.GetCompactLoss();
	}

	for (int i = 0; i < m_polytope_palette.m_cross_edges.size(); i++) {
		Edge4d& e = m_polytope_palette.m_cross_edges[i];
		m_polytope_palette.m_cross_edges[i].update(m_polytope_palette.m_vertices[e.v1_id], m_polytope_palette.m_vertices[e.v2_id], e.v1_id, e.v2_id);
		sloss += m_polytope_palette.m_cross_edges[i].slope;
		//sloss += e.GetSlope();
	}

	rloss /= SysParameter::SampFrameCnt();
	closs /= SysParameter::SampFrameCnt();
	sloss /= m_polytope_palette.m_cross_edges.size();
}


double global_ObjectFunction(unsigned n, const double *x, double *grad, void *data) {
	VideoPalette* pvp = (VideoPalette*)data;
	int vid = pvp->m_curr_refine_vidx;
	pvp->m_polytope_palette.m_vertices[vid][0] = x[0];
	pvp->m_polytope_palette.m_vertices[vid][1] = x[1];
	pvp->m_polytope_palette.m_vertices[vid][2] = x[2];
	pvp->m_polytope_palette.m_vertices[vid][3] = x[3];

	double rloss = 0, closs = 0, sloss = 0;
	double energy = pvp->GetEnergy(rloss, closs, sloss);

	//cout << pvp->m_polytope_palette.m_vertices[vid]<< "  "<<energy<<", rloss:" << rloss << ",closs:" << closs << ", sloss:" << sloss  << endl;
	return energy;
}