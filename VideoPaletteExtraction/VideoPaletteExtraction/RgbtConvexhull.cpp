#include "RgbtConvexhull.h"
#include "libqhullcpp/QhullFacetList.h"
#include "libqhullcpp/Qhull.h"
#include "RgbtChEdgeReduction.h"

using namespace orgQhull;

RgbtConvexhull::RgbtConvexhull(vector<cv::Vec4d>& vertices) {
	m_vertices = vertices;
	BuildRgbtConvexhull();
}

RgbtConvexhull::RgbtConvexhull(Frame& f1, Frame& f2) { 
	vector<cv::Vec3d> vertices_1 = f1.m_cvxhull.m_vertices;
	for (int i = 0; i < vertices_1.size(); i++) { 
		cv::Vec4d v(vertices_1[i][0], vertices_1[i][1], vertices_1[i][2], f1.m_normal_t);
		m_vertices.push_back(v);
	}

	vector<cv::Vec3d> vertices_2 = f2.m_cvxhull.m_vertices;
	for (int i = 0; i < vertices_2.size(); i++) {
		cv::Vec4d v(vertices_2[i][0], vertices_2[i][1], vertices_2[i][2], f2.m_normal_t);
		m_vertices.push_back(v);
	}

	BuildRgbtConvexhull();
}

void RgbtConvexhull::BuildRgbtConvexhull() {
	Qhull qhull_;
	qhull_.runQhull4D(m_vertices, "Qt");

	//simplex of the rgbt convex hull
	m_simplices.clear();
	QhullFacetList facets = qhull_.facetList();
	for (QhullFacetList::iterator it = facets.begin(); it != facets.end(); ++it) {
		if (!(*it).isGood()) continue;
		QhullFacet f = *it;
		QhullVertexSet vSet = f.vertices();

		int fvid[4], k = 0;
		for (QhullVertexSet::iterator vIt = vSet.begin(); vIt != vSet.end(); ++vIt) {
			QhullVertex v = *vIt;
			QhullPoint p = v.point();
			fvid[k++] = p.id();
		}
		m_simplices.push_back(cv::Vec4i(fvid[0], fvid[1], fvid[2], fvid[3]));
	}

	GetCrossEdges();
}

void RgbtConvexhull::GetCrossEdges() {
	set<pair<int, int> > crosst_edges_set;
	for each (cv::Vec4i simplex in m_simplices) {
		for (int j = 0; j < 4; j++) {
			for (int k = j + 1; k < 4; k++) {
				int u_id = simplex[j];
				int v_id = simplex[k];

				double u_t = m_vertices[u_id][3];
				double v_t = m_vertices[v_id][3];
				if (fabs(u_t - v_t) > ESP) {
					int vid_min = min(u_id, v_id);
					int vid_max = max(u_id, v_id);
					crosst_edges_set.insert(make_pair(vid_min, vid_max));
				}
			}
		}
	}
	m_cross_edges.clear();
	for (set<pair<int, int> >::iterator it = crosst_edges_set.begin();
		it != crosst_edges_set.end(); it++) {
		int u_id = it->first;
		int v_id = it->second;
		cv::Vec4d u = m_vertices[u_id];
		cv::Vec4d v = m_vertices[v_id];
		m_cross_edges.push_back(Edge4d(u, v, u_id, v_id));
	}
}

//ch1 and ch2 may not meet the edge correspondence at the beginning (for example, two diagonals in different directions in two rectangles),then relax it
ReMoveStatus RgbtConvexhull::PreRemoveSomeCrossEdges(RgbConvexhull& ch1, RgbConvexhull& ch2) {
	
	const int EXPECT_REMAIN_EDGENUM = 19;

	//0.if cross edge number < EXPECT_REMAIN_EDGENUM, return
	vector<Edge4d> edges_vec = m_cross_edges;
	if (edges_vec.size() <= EXPECT_REMAIN_EDGENUM)
		return PremoveEdgeSuccess;

	//1.sorting all cross edges from long to short
	for (int i = 0; i < edges_vec.size(); i++)
		edges_vec[i].cost = edges_vec[i].rgb_len;
	sort(edges_vec.begin(), edges_vec.end(), cmp_rgb_edge);

	int vert_cnt = ch1.m_vertices.size() + ch2.m_vertices.size();
	RgbtChEdgeReduction EdgeReduce(vert_cnt, ch1.m_edges, ch2.m_edges);

	vector<bool> removed_vids(edges_vec.size(), false);
	int intial_edges_cnt = edges_vec.size();
	int removed_edges_cnt = 0, idx = 0;

	//2.check each edge if it can be removed
	while (1) {
		removed_vids[idx] = true;

		vector<Edge4d> curr_cross_edges;
		for (int i = 0; i < intial_edges_cnt; i++) {
			if (!removed_vids[i])
				curr_cross_edges.push_back(edges_vec[i]);
		}

		removed_vids[idx] = EdgeReduce.CheckConnectionConstraint(curr_cross_edges);
		removed_edges_cnt += removed_vids[idx];
		idx++;

		if (intial_edges_cnt - removed_edges_cnt <= EXPECT_REMAIN_EDGENUM)
			break;

		if (idx >= intial_edges_cnt)
			return PremoveEdgeFail;
	}
	cout << "pre remove: " << removed_edges_cnt << " cross edges" << endl;

	//3.update the edges
	m_cross_edges.clear();
	for (int i = 0; i < intial_edges_cnt; i++) {
		if (!removed_vids[i])
			m_cross_edges.push_back(edges_vec[i]);
	}
	return PremoveEdgeSuccess;
}

//remove redundant edges
ReMoveStatus RgbtConvexhull::RemoveRedundantCrossEdges(RgbConvexhull& ch1, RgbConvexhull& ch2) {
	
	//1.number all vertices sequentially(ch1's vertices -> ch2's vertices)
	int ch1_vert_cnt = ch1.m_vertices.size();
	set<pair<int, int> > new_edges_set;
	for (set<pair<int, int> >::iterator it = ch2.m_edges.begin(); it != ch2.m_edges.end(); it++) {
		int u = it->first + ch1_vert_cnt;
		int v = it->second + ch1_vert_cnt;
		new_edges_set.insert(make_pair(u, v));
	}
	ch2.m_edges = new_edges_set;
	
	
	vector<Edge4d> best_crosst_edges;
	int before_edge_cnt = m_cross_edges.size();
	int toal_vert_cnt = ch1.m_vertices.size() + ch2.m_vertices.size();

	RgbtChEdgeReduction EdgeReduce(toal_vert_cnt, ch1.m_edges, ch2.m_edges);

	//2. pre-remove some long edges, if failed, relax
	ReMoveStatus pre_remove_stat = PreRemoveSomeCrossEdges(ch1, ch2);

	//2.1 if failed, return
	if (PremoveEdgeFail == pre_remove_stat)
		return PremoveEdgeFail;

	//2.2 if successful, remove more redundant edges
	if (PremoveEdgeSuccess ==  pre_remove_stat) {
		best_crosst_edges = EdgeReduce.FindBestConnectivity(m_cross_edges);

		if (best_crosst_edges.size() == 0)
			return FinalRemoveEdgeFail;
	}

	m_cross_edges = best_crosst_edges;

	//debug
	cout <<int(m_vertices.front()[3] * SysParameter::FrameCnt())<<"->"<< int(m_vertices.back()[3] * SysParameter::FrameCnt()) << " total remove: " << before_edge_cnt - best_crosst_edges.size() << " cross edges, " <<
		best_crosst_edges.size() << " edges left. loss = " << EdgeReduce.m_best_cost<< endl;

	for (int i = 0; i < best_crosst_edges.size(); i++) {
		cout << best_crosst_edges[i].v1_id  << " ," << best_crosst_edges[i].v2_id  << endl;
	}
	cout <<"=============================================================="<< endl;

	return FinalRemoveEdgeSuccess;
}

void RgbtConvexhull::ForceEdgeConnect(RgbConvexhull& ch1, RgbConvexhull& ch2) {

	vector<cv::Vec3d> ch1_verts = ch1.m_vertices;
	vector<cv::Vec3d> ch2_verts = ch2.m_vertices;

	double t1 = m_vertices.front()[3];
	double t2 = m_vertices.back()[3];
	m_cross_edges.clear();

	vector<bool> visited(ch2_verts.size(), false);
	for (int i = 0; i < ch1_verts.size(); i++) {

		cv::Vec4d vi(ch1_verts[i][0], ch1_verts[i][1], ch1_verts[i][2], t1);
		double min_dis = DBL_MAX;
		int min_edge_jid = 0;
		Edge4d min_edge;

		for (int j = 0; j < ch2_verts.size(); j++) {
			if (visited[j])
				continue;

			cv::Vec4d vj(ch2_verts[j][0], ch2_verts[j][1], ch2_verts[j][2], t2);
			int global_jid = j + ch1_verts.size();
			Edge4d e(vi, vj, i, global_jid);

			if (e.len < min_dis) {
				min_dis = e.len;
				min_edge = e;
				min_edge_jid = j;
			}
		}

		visited[min_edge_jid] = true;
		m_cross_edges.push_back(min_edge);
	}

	/*
	for (int j = 0; j < ch2_verts.size(); j++) {
		if (visited[j] == false) {
			cv::Vec4d vj(ch2_verts[j][0], ch2_verts[j][1], ch2_verts[j][2], t2);
			double min_dis = DBL_MAX;
			Edge4d min_edge;
			int global_jid = j + ch1_verts.size();

			for (int i = 0; i < ch1_verts.size(); i++) {
				cv::Vec4d vi(ch1_verts[i][0], ch1_verts[i][1], ch1_verts[i][2], t1);
				Edge4d e(vi, vj, i, global_jid);
				if (e.len < min_dis) {
					min_dis = e.len;
					min_edge = e;
				}
			}
			visited[j] = true;
			m_cross_edges.push_back(min_edge);
		}
	}
	*/
}