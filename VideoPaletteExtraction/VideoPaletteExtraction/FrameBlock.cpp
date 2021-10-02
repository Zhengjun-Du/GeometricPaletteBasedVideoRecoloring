#include "FrameBlock.h"
#include <fstream>
#include <omp.h>
#include <numeric>

static omp_lock_t locker;

FrameBlock::FrameBlock(vector<Frame>& frames, vector<RgbtConvexhull>& adj_rgbt_chs) {
	m_frames = frames;
	m_rgbt_cvxhulls = adj_rgbt_chs;
}

//group the adjacent continuous frames into a block
void FrameBlock::BuildInitialFrameBlocks() {
	m_frm_blocks.clear();
	int start = 0, end = 0; 
	for (int i = 0; i < m_frames.size() - 1; i++) {
		int frm1_vert_cnt = m_frames[i].m_cvxhull.m_vertices.size();
		int frm2_vert_cnt = m_frames[i + 1].m_cvxhull.m_vertices.size();

		if (frm1_vert_cnt == frm2_vert_cnt &&
			frm2_vert_cnt == m_rgbt_cvxhulls[i].m_cross_edges.size()) {
			end++;
		}
		else {
			vector<int> ids;
			for (int j = start; j <= end; j++)
				ids.push_back(j);
			m_frm_blocks.push_back(ids);

			start = end = end + 1;
		}
	}

	vector<int> ids;
	for (int j = start; j <= end; j++)
		ids.push_back(j);
	m_frm_blocks.push_back(ids);

	//2.block record each frame's block
	for (int i = 0; i < m_frm_blocks.size(); i++) {
		for (int j = 0; j < m_frm_blocks[i].size(); j++) {
			int frm_id = m_frm_blocks[i][j];
			m_frames[frm_id].m_blk_id = i;
		}
	}

	//output the initial block info
	std::cout << "intitial video block are: " << endl;
	for (int i = 0; i < m_frm_blocks.size(); i++) {
		std::cout << i << ": ";
		for (int j = 0; j < m_frm_blocks[i].size(); j++)
			std::cout << m_frm_blocks[i][j] << " ";
		std::cout << endl;
	}
}

double FrameBlock::ComputeOverallReconstructAndCompactLoss() {
	double sum_rc_loss = 0;
	double sum_rc_w = 0;

	omp_init_lock(&locker);
	#pragma omp parallel for num_threads(32)
	for (int i = 0; i < m_frm_blocks.size(); i++) {

		string block_str = block2String(m_frm_blocks[i]);
		if (m_block_rcloss_map.find(block_str) != m_block_rcloss_map.end())
			continue;

		vector<Frame> block_frames;
		for (int j = 0; j < m_frm_blocks[i].size(); j++) {
			int fid = m_frm_blocks[i][j];
			block_frames.push_back(m_frames[j]);
		}
		cv::Vec3d w_rloss_closs = ComputeSingleBlockReconstructAndCompactLoss(block_frames);
		double rc_w_i = w_rloss_closs[0];
		double r_loss_i = w_rloss_closs[1];
		double c_loss_i = w_rloss_closs[2];

		omp_set_lock(&locker);		// lock

		m_block_rcloss_map[block_str] = w_rloss_closs;
		sum_rc_loss += rc_w_i * (r_loss_i + c_loss_i);
		sum_rc_w += rc_w_i *  m_frm_blocks[i].size();

		omp_unset_lock(&locker);	// unlock
	}
	omp_destroy_lock(&locker);

	//double toal_rc_loss = sum_rc_loss / sum_rc_w;
	double toal_rc_loss = sum_rc_loss / SysParameter::FrameCnt();
	return toal_rc_loss;
}


void FrameBlock::MergeFrameBlocks() {

	assert(!m_frm_blocks.empty());

	//0. output the initial skew polytope
	int merge_id = 0;
	GenerateSkewPolyhedron_coarse();

	//1. initial loss
	double total_rc_loss = ComputeOverallReconstructAndCompactLoss();
	double initial_loss = total_rc_loss;

	vector<double> block_merge_losses;
	block_merge_losses.push_back(initial_loss);

	std::cout << endl << "initia loss: " << initial_loss << endl;
	std::cout << endl << "start to merge blocks..." << endl;

	//2. merge blocks
	while (m_frm_blocks.size() > 1) {

		//2.1 re calculate the reconstruction and compactness loss
		ComputeOverallReconstructAndCompactLoss();

		//2.2 calculate every adjacent blocks' mergeing loss
		omp_init_lock(&locker);
		vector<BlockMergeLoss> merge_losses(2 * (m_frm_blocks.size() - 1));

		#pragma omp parallel for num_threads(32)
		for (int i = 0; i < m_frm_blocks.size() - 1; i++) {
			double rloss = 0, closs = 0, sloss = 0;

			if (m_frm_blocks.size() > 2) {
				//the loss of merge block i - > i + 1
				double loss1 = MergeAdjacentBlocks(i, i + 1, rloss, closs);
				merge_losses[2 * i] = BlockMergeLoss(loss1, rloss, closs, i, i + 1);

				//the loss of merge block i + 1 -> i
				double loss2 = MergeAdjacentBlocks(i + 1, i, rloss, closs);
				merge_losses[2 * i + 1] = BlockMergeLoss(loss2, rloss, closs, i + 1, i);
			}

			//if only 2 block left，calculate the loss of smaller block -> bigger block 
			else {
				if (m_frm_blocks[i].size() <= m_frm_blocks[i + 1].size()) {
					double loss1 = MergeAdjacentBlocks(i, i + 1, rloss, closs);
					merge_losses[2 * i] = BlockMergeLoss(loss1, rloss, closs, i, i + 1);
				}
				else {
					double loss2 = MergeAdjacentBlocks(i + 1, i, rloss, closs);
					merge_losses[2 * i] = BlockMergeLoss(loss2, rloss, closs, i + 1, i);
				}
				merge_losses[2 * i + 1] = merge_losses[2 * i];
			}
		}
		omp_destroy_lock(&locker);

		//2.3 get the block ids with minimal merging loss 
		sort(merge_losses.begin(), merge_losses.end());
		double best_loss = merge_losses[0].loss;
		double best_rloss = merge_losses[0].rloss;
		double best_closs = merge_losses[0].closs;
		int best_src_id = merge_losses[0].src_blk_id;
		int best_tar_id = merge_losses[0].tar_blk_id;
		block_merge_losses.push_back(best_loss);

		cout << "min loss: " << best_loss << ",  src_block_id:" << best_src_id << ", tar_block_id:" << best_tar_id << endl;

		//**termination condition
		if (best_loss > SysParameter::BlockTh() * initial_loss) {
			break;
		}

		//2.4 update the RGB convex hulls of each frame in the best_src_id block
		int min_frm_index = min4(m_frm_blocks[best_src_id].front(), m_frm_blocks[best_tar_id].front(), m_frm_blocks[best_src_id].back(), m_frm_blocks[best_tar_id].back());
		int max_frm_index = max4(m_frm_blocks[best_src_id].front(), m_frm_blocks[best_tar_id].front(), m_frm_blocks[best_src_id].back(), m_frm_blocks[best_tar_id].back());
		
		//clear map
		string block_merge_str1 = to_string(min_frm_index) + "-" + to_string(max_frm_index);
		string block_merge_str2 = to_string(max_frm_index) + "-" + to_string(min_frm_index);
		string block_merge_str = best_src_id < best_tar_id ? block_merge_str1 : block_merge_str2;
		vector<Frame> changed_frms = m_block_updated_frames_map[block_merge_str];
		m_block_updated_frames_map.erase(block_merge_str1);
		m_block_updated_frames_map.erase(block_merge_str2);

		//clear map
		cv::Vec3d w_rloss_closs1 = m_block_rcloss_map[block_merge_str1];
		cv::Vec3d w_rloss_closs2 = m_block_rcloss_map[block_merge_str2];
		string block_frm_str = "BF-" + to_string(min_frm_index) + "-" + to_string(max_frm_index);
		m_block_rcloss_map[block_frm_str] = best_src_id < best_tar_id ? w_rloss_closs1 : w_rloss_closs2;

		m_block_rcloss_map.erase(block_merge_str1);
		m_block_rcloss_map.erase(block_merge_str2);
		m_block_rcloss_map.erase(block2String(m_frm_blocks[best_src_id]));
		m_block_rcloss_map.erase(block2String(m_frm_blocks[best_tar_id]));

		int minBlockId = min(best_src_id, best_tar_id);
		int maxBlockId = max(best_src_id, best_tar_id);
		int before_min_block_id = minBlockId - 1;
		int after_max_block_id = maxBlockId + 1;

		//clear map
		if (0 <= before_min_block_id) {
			int minFrmIndex = m_frm_blocks[before_min_block_id].front();
			int maxFrmIndex = m_frm_blocks[minBlockId].back();
			string blockMergestr1 = to_string(minFrmIndex) + "-" + to_string(maxFrmIndex);
			string blockMergestr2 = to_string(maxFrmIndex) + "-" + to_string(minFrmIndex);
			m_block_updated_frames_map.erase(blockMergestr1);
			m_block_updated_frames_map.erase(blockMergestr2);
			m_block_rcloss_map.erase(blockMergestr1);
			m_block_rcloss_map.erase(blockMergestr2);
		}
		if (after_max_block_id < m_frm_blocks.size()) {
			int minFrmIndex = m_frm_blocks[maxBlockId].front();
			int maxFrmIndex = m_frm_blocks[after_max_block_id].back();
			string blockMergestr1 = to_string(minFrmIndex) + "-" + to_string(maxFrmIndex);
			string blockMergestr2 = to_string(maxFrmIndex) + "-" + to_string(minFrmIndex);
			m_block_updated_frames_map.erase(blockMergestr1);
			m_block_updated_frames_map.erase(blockMergestr2);
			m_block_rcloss_map.erase(blockMergestr1);
			m_block_rcloss_map.erase(blockMergestr2);
		}

		if (!changed_frms.empty()) {
			for (int j = 0; j < m_frm_blocks[best_src_id].size(); j++) {
				int frame_id = m_frm_blocks[best_src_id][j];
				m_frames[frame_id] = changed_frms[j];
			}
		}

		//update range(noted that: 2.5 and 2.6 cannot be exchanged)
		int beg_fid = max(0, m_frm_blocks[best_src_id].front() - 1);  //the last frame of previous block
		int end_fid = min(m_frm_blocks[best_src_id].back() + 1, (int)m_frames.size() - 1); //the first frame of the next block

		//2.5 update m_frm_blocks
		int min_id = min(best_src_id, best_tar_id);
		int max_id = max(best_src_id, best_tar_id);
		m_frm_blocks[min_id].insert(m_frm_blocks[min_id].end(), m_frm_blocks[max_id].begin(), m_frm_blocks[max_id].end());
		m_frm_blocks.erase(m_frm_blocks.begin() + max_id);

		//2.6 update the frames' block id
		for (int i = 0; i < m_frm_blocks.size(); i++) {
			for (int j = 0; j < m_frm_blocks[i].size(); j++) {
				int frm_id = m_frm_blocks[i][j];
				m_frames[frm_id].m_blk_id = i;
			}
		}

		//2.7 update the skew polytope from beg_fid to end_fid
		UpdateSkewPolyhedronTopologyWithin(beg_fid, end_fid);

		/*
		bool block_update = false;
		for (int k = m_frm_blocks.size() - 2; k >= 0; k--) {
			int leftFrmId = m_frm_blocks[k].back();
			Frame leftFrm = m_frames[leftFrmId];
			Frame rightFrm = m_frames[leftFrmId + 1];
			if (m_rgbt_cvxhulls[leftFrmId].m_cross_edges.size() == leftFrm.m_cvxhull.m_vertices.size() &&
				leftFrm.m_cvxhull.m_vertices.size() == rightFrm.m_cvxhull.m_vertices.size()) {
				m_frm_blocks[k].insert(m_frm_blocks[k].end(), m_frm_blocks[k + 1].begin(), m_frm_blocks[k + 1].end());
				m_frm_blocks.erase(m_frm_blocks.begin() + k + 1);
				block_update = true;
			}
		}
		if (block_update) {
			for (int i = 0; i < m_frm_blocks.size(); i++) {
				for (int j = 0; j < m_frm_blocks[i].size(); j++) {
					int frm_id = m_frm_blocks[i][j];
					m_frames[frm_id].m_blk_id = i;
				}
			}
		}*/


		//2.8 output debug info
		std::cout <<" after merge:" << endl;
		for (int i = 0; i < m_frm_blocks.size(); i++) {
			std::cout << i << ": ";
			for (int j = 0; j < m_frm_blocks[i].size(); j++)
				std::cout << m_frm_blocks[i][j] << " ";
			std::cout << endl;
		}
		std::cout << "merge blocks: " << best_src_id << " -> " << best_tar_id << " loss: " << best_loss << "\t initial loss:" << initial_loss << endl;

		merge_id++;
		GenerateSkewPolyhedron_coarse();

		//ComputeAveFrameLoss(R, C);
	}
	//3.generare the skew polyhetope(contains vertex, edge, corresponding topologies，adjacent vertices)
	GenerateSkewPolyhedron_fine();


	std::cout << endl << "end to merge blocks..." << endl;

	m_block_updated_frames_map.clear();
	m_block_rcloss_map.clear();


	//4.correct each frame covex hull's face normal
	for (int i = 0; i < m_frames.size(); i++)
		m_frames[i].m_cvxhull.CorrectNormal(); 
}

//merge 2 adjacent blocks a and b
double FrameBlock::MergeAdjacentBlocks(int a, int b, double& total_rloss, double& total_closs) {
	total_rloss = total_closs = 0;
	vector<float> losses_except_ab(m_frm_blocks.size(), 0);

	//1. other blocks'(without a and b) rloss and closs
	for (int i = 0; i < m_frm_blocks.size(); i++) {
		if (i != a && i != b) {
			string block_str = block2String(m_frm_blocks[i]);
			double rc_w = m_block_rcloss_map[block_str][0];
			double rloss = m_block_rcloss_map[block_str][1];
			double closs = m_block_rcloss_map[block_str][2];

			total_rloss += rc_w * rloss;
			total_closs += rc_w * closs;
		}
	}

	//2.check if the loss of (block a,b) has been calculated
	int min_frm_index = min4(m_frm_blocks[a].front(), m_frm_blocks[b].front(), m_frm_blocks[a].back(), m_frm_blocks[b].back());
	int max_frm_index = max4(m_frm_blocks[a].front(), m_frm_blocks[b].front(), m_frm_blocks[a].back(), m_frm_blocks[b].back());
	string block_merge_str = to_string(min_frm_index) + "-" + to_string(max_frm_index);
	if(a > b) block_merge_str = to_string(max_frm_index) + "-" + to_string(min_frm_index);

	if (m_block_rcloss_map.find(block_merge_str) != m_block_rcloss_map.end()) {
		double rc_w = m_block_rcloss_map[block_merge_str][0];
		double rloss = m_block_rcloss_map[block_merge_str][1];
		double closs = m_block_rcloss_map[block_merge_str][2];
		total_rloss += rc_w * rloss;
		total_closs += rc_w * closs;
		total_rloss /= SysParameter::FrameCnt();
		total_closs /= SysParameter::FrameCnt();
		double total_rc_loss = total_rloss + total_closs;

		std::cout << "computed total_rcs_loss: merge block " << a << " to block " << b << " : " << total_rc_loss << endl;
		return total_rc_loss;
	}

	vector<Frame> merged_blkab_frames;
	vector<Frame> updated_blka_frames(m_frm_blocks[a].size()); //ensure the relative order

	//2.1 blk_a -> block b: block a (iteration from front to back)：ConvexHull_s <- ConvexHull_s-1, and then refine
	if (b < a) {
		int lastFidOfB = m_frm_blocks[b].back();
		Frame previous_frm = m_frames[lastFidOfB];

		for (int i = 0; i < m_frm_blocks[a].size(); i++) {
			int frame_id = m_frm_blocks[a][i];
			Frame temp_frm = m_frames[frame_id];

			temp_frm.m_cvxhull = previous_frm.m_cvxhull;
			temp_frm.Refine();

			updated_blka_frames[i] = temp_frm;
			merged_blkab_frames.push_back(temp_frm);

			previous_frm = temp_frm;
		}
	}

	//2.2 blk_a -> block b: block a (iteration from back to front)：ConvexHull_s <- ConvexHull_s+1, and then refine
	else {
		int firstFidOfB = m_frm_blocks[b].front();
		Frame next_frm = m_frames[firstFidOfB];

		for (int i = m_frm_blocks[a].size() - 1; i >= 0; i--) {
			int frame_id = m_frm_blocks[a][i];
			Frame temp_frm = m_frames[frame_id];

			temp_frm.m_cvxhull = next_frm.m_cvxhull;
			temp_frm.Refine();

			updated_blka_frames[i] = temp_frm;
			merged_blkab_frames.push_back(temp_frm);

			next_frm = temp_frm;
		}
	}

	//3. block b
	int ab_frm_cnt = m_frm_blocks[a].size() + m_frm_blocks[b].size();
	for (int i = 0; i < m_frm_blocks[b].size(); i++) {
		int fid = m_frm_blocks[b][i];
		Frame frame = m_frames[fid];
		merged_blkab_frames.push_back(frame);
	}

	//4. the rloss and closs after merging  block a and b 
	cv::Vec3d w_rloss_closs = ComputeSingleBlockReconstructAndCompactLoss(merged_blkab_frames);
	double rc_w = w_rloss_closs[0];
	double rloss = w_rloss_closs[1];
	double closs = w_rloss_closs[2];

	total_rloss += rc_w * rloss;
	total_closs += rc_w * closs;

	total_rloss /= SysParameter::FrameCnt();
	total_closs /= SysParameter::FrameCnt();
	double total_rc_loss = total_rloss + total_closs;

	//cout << total_rloss << "\t" << total_closs << endl;

	//5. save some 
	omp_set_lock(&locker);		//lock

	m_block_updated_frames_map[block_merge_str] = updated_blka_frames;

	//5.1 update loss when block a merges to block b
	m_block_rcloss_map[block_merge_str] = w_rloss_closs;
	std::cout << "total_rcs_loss: merge block " << a << " to block " << b << " : " << total_rc_loss << endl;

	omp_unset_lock(&locker);	//unlock

	return total_rc_loss;
}

cv::Vec3d FrameBlock::ComputeSingleBlockReconstructAndCompactLoss(vector<Frame>& block_frms) {
	int blk_frms_cnt = block_frms.size();
	int each_frm_vert_cnt = block_frms[0].m_cvxhull.m_vertices.size();
	double sum_rloss = 0;
	double sum_closs = 0;

	for (int j = 0; j < blk_frms_cnt; j++) {
		Frame frame = block_frms[j];
		frame.GetLoss();
		sum_rloss += SysParameter::BlockRlossWt() * frame.m_rloss;
		sum_closs += SysParameter::BlockClossWt() * frame.m_closs;
	}

	double w = (1 + SysParameter::BlockAlpha() * 1.0 * blk_frms_cnt / SysParameter::FrameCnt())*(1 + SysParameter::BlockBeta() * each_frm_vert_cnt);
	return cv::Vec3d(w, sum_rloss, sum_closs);
}

//recompute the RGBT convex hull and simplify it
void FrameBlock::UpdateSkewPolyhedronTopologyWithin(int beg_fid, int end_fid) {
	std::cout << endl << "rebuild " << beg_fid << "->" << end_fid << endl << endl;

	#pragma omp parallel for num_threads(32)
	for (int i = beg_fid; i < end_fid; i++) {
		m_rgbt_cvxhulls[i] = RgbtConvexhull(m_frames[i], m_frames[i + 1]);
		RgbConvexhull ch1 = m_frames[i].m_cvxhull;
		RgbConvexhull ch2 = m_frames[i + 1].m_cvxhull;

		//1. if frame i and i+1 are in the same block
		if (m_frames[i].m_blk_id == m_frames[i + 1].m_blk_id) {
			m_rgbt_cvxhulls[i].m_cross_edges.clear();
			int  k = ch1.m_vertices.size();
			for (int j = 0; j < ch1.m_vertices.size(); j++, k++) {
				Edge4d e(m_rgbt_cvxhulls[i].m_vertices[j], m_rgbt_cvxhulls[i].m_vertices[k], j, k);
				m_rgbt_cvxhulls[i].m_cross_edges.push_back(e);
			}
		}

		//2. if frame i and i+1 are in different blocks, error
		else {
			ReMoveStatus remove_stat = m_rgbt_cvxhulls[i].RemoveRedundantCrossEdges(ch1, ch2);
			if (FinalRemoveEdgeSuccess != remove_stat) {
				m_rgbt_cvxhulls[i].ForceEdgeConnect(ch1, ch2);
				cout << "ERROR: in adjacnet blocks(" << m_frames[i].m_blk_id << "," << m_frames[i + 1].m_blk_id << "), adjacnt frames (" << i << "," << i + 1 << ") can not connect one-by-one!" << endl;
			}
		}
		//cout << "rgbtch id:" << i << "\t ch1 vcnt:" << ch1.m_vertices.size() << "\t ch2 vcnt:" << ch2.m_vertices.size() << "\t ecnt:" << m_rgbt_cvxhulls[i].m_cross_edges.size() << endl;
	}
}


//generate the coarse skew polytope（only contain vertex and edge）
void FrameBlock::GenerateSkewPolyhedron_coarse() {

	m_skew_polytope.m_vertices.clear();
	m_skew_polytope.m_cross_edges.clear();
	m_skew_polytope.m_vert_adjverts.clear();

	//1. add vertex
	vector<Frame>& frms = m_frames;
	for (int i = 0; i < frms.size(); i++) {
		vector<cv::Vec3d>& vertices = frms[i].m_cvxhull.m_vertices;
		for (int j = 0; j < vertices.size(); j++) {
			cv::Vec4d v(vertices[j][0], vertices[j][1], vertices[j][2], frms[i].m_normal_t);
			m_skew_polytope.m_vertices.push_back(v);
		}
	}

	//2.add inter frame edges
	int vcnt = 0;
	for (int i = 0; i < m_rgbt_cvxhulls.size(); i++) {
		vector<Edge4d> edges = m_rgbt_cvxhulls[i].m_cross_edges;
		for (int j = 0; j < edges.size(); j++) {
			int uid = edges[j].v1_id + vcnt;
			int vid = edges[j].v2_id + vcnt;
			int min_id = min(uid, vid), max_id = max(uid, vid);
			Edge4d e(edges[j].v1, edges[j].v2, uid, vid);

			/*
			if (fabs(edges[j].v1[0] - m_skew_polytope.m_vertices[uid][0]) > 0.0001) {
				cout << "error:" << i << " " << j << " " << uid << " " << vid << endl;
				cout << edges[j].v1 << endl;
				cout << edges[j].v2 << endl;
			}
			*/

			m_skew_polytope.m_cross_edges.push_back(e);
		}
		vcnt += m_frames[i].m_cvxhull.m_vertices.size();
	}
}

//generate the fine skew polytope(contains vertex, edge, corresponding topologies，adjacent vertices)
void FrameBlock::GenerateSkewPolyhedron_fine() {
	cout << "\nGenerateSkewPolyhedron_fine" << endl;
	m_skew_polytope.m_vertices.clear();
	m_skew_polytope.m_cross_edges.clear();
	m_skew_polytope.m_vert_adjverts.clear();

	//1.add vertex
	vector<Frame>& frms = m_frames;
	for (int i = 0; i < frms.size(); i++) {
		vector<cv::Vec3d>& vertices = frms[i].m_cvxhull.m_vertices;
		for (int j = 0; j < vertices.size(); j++) {
			cv::Vec4d v(vertices[j][0], vertices[j][1], vertices[j][2], frms[i].m_normal_t);
			m_skew_polytope.m_vertices.push_back(v);
		}
	}

	//2.record whether the frame is the first frame of some block
	vector<bool> is_first_frame_of_some_block(m_frames.size(), false);
	for (int i = 0; i < m_frm_blocks.size(); i++) {
		int frm_id = m_frm_blocks[i][0];
		is_first_frame_of_some_block[frm_id] = true;
	}

	//3.record the topology of the first frame of each block
	int vcnt = 0;
	map<int, int> currVert_to_BlockHeadChVidx;
	for (int i = 0; i < m_rgbt_cvxhulls.size(); i++) {
		vector<Edge4d> edges = m_rgbt_cvxhulls[i].m_cross_edges;

		//record the vertex indices of the first frame in the block
		if (is_first_frame_of_some_block[i]) {
			for (int j = 0; j < m_frames[i].m_cvxhull.m_vertices.size(); j++) {
				int uid = j + vcnt;
				currVert_to_BlockHeadChVidx[uid] = j;
			}
		}
		vcnt += m_frames[i].m_cvxhull.m_vertices.size();
	}

	//4.in the skew polyhderon,move along the cross edge chain to the first frame of the current block, find the vertex id
	vcnt = 0;
	for (int i = 0; i < m_rgbt_cvxhulls.size(); i++) {
		vector<Edge4d> edges = m_rgbt_cvxhulls[i].m_cross_edges;
		int curr_blk = m_frames[i].m_blk_id;
		int next_blk = m_frames[i + 1].m_blk_id;

		for (int j = 0; j < edges.size(); j++) {
			int uid = edges[j].v1_id + vcnt;
			int vid = edges[j].v2_id + vcnt;
			int min_id = min(uid, vid), max_id = max(uid, vid);

			//the vertex id of the first frame in the current block corresponding to the edge(uid,vid)
			map<int, int> mp;
			mp[curr_blk] = currVert_to_BlockHeadChVidx[uid];

			//4.1 if frame i and frame i + 1 are in the same block
			if (curr_blk == m_frames[i + 1].m_blk_id) {
				//update vid，from front to back
				currVert_to_BlockHeadChVidx[vid] = currVert_to_BlockHeadChVidx[uid];
			}

			//4.2 if frame i and frame i + 1 are in different blocks
			else {
				//the vertex id of the first frame in next block corresponding to the edge(uid,vid)
				mp[next_blk] = currVert_to_BlockHeadChVidx[vid];
			}

			m_skew_polytope.m_cross_edges.push_back(Edge4d(edges[j].v1, edges[j].v2, uid, vid, mp));
			m_skew_polytope.m_vert_adjverts[min_id].insert(max_id);
			m_skew_polytope.m_vert_adjverts[max_id].insert(min_id);
		}
		vcnt += m_frames[i].m_cvxhull.m_vertices.size();
	}
}

void FrameBlock::ComputeAveFrameLoss(double& rloss, double& closs) {
	rloss = closs = 0;

	for (int i = 0; i < m_frames.size(); i++) {
		double rloss_ = m_frames[i].GetReconstuctLoss();
		double closs_ = m_frames[i].GetCompactLoss();
		rloss += rloss_;
		closs += closs_;
	}

	rloss /= SysParameter::SampFrameCnt();
	closs /= SysParameter::SampFrameCnt();
}