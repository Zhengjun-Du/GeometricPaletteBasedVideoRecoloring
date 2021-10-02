#include "RgbtSkewPolytope.h"
#include "utility.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <set>

using namespace std;
using namespace cv;


RgbConvexhull RgbtSkewPolytope::SlicePolyhedronForFrame(int fid, bool record_slice_info) {

	//frames in the same block have the same topology, it is the simplex in the first frame
	int blk_id = m_pframes->at(fid).m_blk_id;
	int head_frm_id = m_frm_blocks[blk_id][0];
	RgbConvexhull rgb_poly = m_pframes->at(head_frm_id).m_cvxhull;

	double t = m_pframes->at(fid).m_normal_t;

	//visit all cross edges
	for (int i = 0; i < m_cross_edges.size(); i++) {

		int v1_id = m_cross_edges[i].v1_id;
		int v2_id = m_cross_edges[i].v2_id;

		//the blocks associated with the current edge and the slice topology information in each block
		map<int, int> blk_vertid_maps = m_cross_edges[i].blk_vertid_maps;

		cv::Vec4d v_1 = m_vertices[v1_id];
		cv::Vec4d v_2 = m_vertices[v2_id];

		double t1 = v_1[3];
		double t2 = v_2[3];

		double t_min = min(t1, t2);
		double t_max = max(t1, t2);

		if (t_max - t_min < ESP)
			continue;

		double m_w1, m_w2;
		if (t_min - ESP < t && t < t_max + ESP) {
			if (abs(t2 - t1) > 1e-6) {
				m_w1 = (t - t1) / (t2 - t1);
				m_w2 = 1 - m_w1;
			}
			else {
				m_w1 = 0.5, m_w2 = 0.5;
			}

			cv::Vec4d new_vert4d = v_1 * m_w2 + v_2 * m_w1;
			cv::Vec3d new_vert3d(new_vert4d[0], new_vert4d[1], new_vert4d[2]);

			//the block corresponding to current frame
			int curr_blk = m_pframes->at(fid).m_blk_id;

			//vertex corresponding to the current edge
			int incident_vert_id = blk_vertid_maps[curr_blk];

			//modify current vertex
			rgb_poly.m_vertices[incident_vert_id] = new_vert3d;

			//record the edge corresponding to the current vertex and its interpolation weight
			string fvs = frameVert2String(fid, new_vert3d);
			m_slicevert_edgeAndWt_map[fvs] = make_pair(i, m_w1);
		}
	}
	return rgb_poly;
}


bool  RgbtSkewPolytope::SlicePolyhedronForAllFrames() {
	m_sliced_polys.clear();
	m_slicevert_edgeAndWt_map.clear();

	for (int fid = 0; fid < m_pframes->size(); fid++) {
		RgbConvexhull ch3d = SlicePolyhedronForFrame(fid, true);
		m_sliced_polys.push_back(ch3d);
	}
	return true;
}

void RgbtSkewPolytope::WriteSlicedPolyhedrons(string path) {
	assert(!m_sliced_polys.empty());
	for (int i = 0; i < m_sliced_polys.size(); i++) {
		string filename = cv::format(path.c_str(), i);
		m_sliced_polys[i].WriteConvexhull(filename);
	}
}

void RgbtSkewPolytope::TryRemoveVertex(int vid) {

	//1.find the edge at right and next to vid
	Edge4d* right_edge = nullptr;
	int right_edge_id = FindEdgeWithLeftVidAs(vid);
	if (-1 != right_edge_id) {
		right_edge = &m_cross_edges[right_edge_id];
	}
	assert(right_edge != nullptr);

	//2.remove vid
	vector<Edge4d> cross_edges;
	for (int i = 0; i < m_cross_edges.size(); i++) {
		Edge4d e = m_cross_edges[i];
		int x = e.v1_id;
		int y = e.v2_id;

		//2.2 current edge e = (x,y) y = vid
		if (y == vid) {
			//2.2.1 update cross edge
			y = right_edge->v2_id;

			//2.2.2 update the topology associated with the right edge
			map<int, int> mp = e.blk_vertid_maps;
			for (map<int, int>::iterator it = right_edge->blk_vertid_maps.begin(); it != right_edge->blk_vertid_maps.end(); it++) {
				mp[it->first] = it->second;
			}
			m_cross_edges[i] = Edge4d(m_vertices[x], m_vertices[y], x, y, mp);

			break;
		}
	}

	m_cross_edges.erase(m_cross_edges.begin() + right_edge_id);
}

void RgbtSkewPolytope::RemoveVertex(int vid) {

	//1.find the edge at right and next to vid
	Edge4d* right_edge = nullptr;
	int right_edge_id = FindEdgeWithLeftVidAs(vid);
	if (-1 != right_edge_id) {
		right_edge = &m_cross_edges[right_edge_id];
	}
	else
		return;

	assert(right_edge != nullptr);

	//2.remove vid
	int new_edge_v1 = 0, new_edge_v2 = 0;
	for (int i = 0; i < m_cross_edges.size(); i++) {
		Edge4d e = m_cross_edges[i];
		int x = e.v1_id;
		int y = e.v2_id;

		//2.2 current edge e = (x,y) y = vid
		if (y == vid) {

			//2.2.1 update cross edge
			y = right_edge->v2_id;

			//2.2.2 update the topology associated with the right edge
			map<int, int> mp = e.blk_vertid_maps;
			for (map<int, int>::iterator it = right_edge->blk_vertid_maps.begin(); it != right_edge->blk_vertid_maps.end(); it++) {
				mp[it->first] = it->second;
			}

			//2.2.3 update cross edge
			m_cross_edges[i] = Edge4d(m_vertices[x], m_vertices[y], x, y, mp);

			//2.2.4 update x's neighbor
			set<int> x_curr_neighbors;
			for (set<int>::iterator it = m_vert_adjverts[x].begin(); it != m_vert_adjverts[x].end(); it++) {
				if (*it != e.v2_id) x_curr_neighbors.insert(*it);
				else x_curr_neighbors.insert(y);
			}
			m_vert_adjverts[x] = x_curr_neighbors;

			//2.2.5 update y's neighbor
			set<int> y_curr_neighbors;
			for (set<int>::iterator it = m_vert_adjverts[y].begin(); it != m_vert_adjverts[y].end(); it++) {
				if (*it != vid) y_curr_neighbors.insert(*it);
				else y_curr_neighbors.insert(x);
			}
			m_vert_adjverts[y] = y_curr_neighbors;

			new_edge_v1 = x;
			new_edge_v2 = y;

			break;
		}
	}

	m_cross_edges.erase(m_cross_edges.begin() + right_edge_id);

	//3.update m_spv_proque due to the distance of some vertices to its opposite edges are changed
	vector<PolyVertex4D> svs;
	while (!m_spv_proque.empty()) {
		PolyVertex4D sv = m_spv_proque.top();
		m_spv_proque.pop();

		if (sv.vid == vid)
			continue;

		else if (sv.vid == new_edge_v1) {
			int vid_left = m_cross_edges[FindEdgeWithRightVidAs(sv.vid)].v1_id;
			int vid_right = new_edge_v2;
			sv.len = DisOfMidVert2OppsiteEdge(m_vertices[vid_left], m_vertices[sv.vid], m_vertices[vid_right]);
		}
		else if (sv.vid == new_edge_v2) {
			int vid_left = new_edge_v1;
			int vid_right = m_cross_edges[FindEdgeWithLeftVidAs(sv.vid)].v2_id;
			sv.len = DisOfMidVert2OppsiteEdge(m_vertices[vid_left], m_vertices[sv.vid], m_vertices[vid_right]);
		}
		svs.push_back(sv);
	}

	for (int i = 0; i < svs.size(); i++)
		m_spv_proque.push(svs[i]);

	cout << "remove vertex: " << vid << endl;
}

int RgbtSkewPolytope::FindEdgeWithLeftVidAs(int id) {
	for (int i = 0; i < m_cross_edges.size(); i++)
		if (m_cross_edges[i].v1_id == id)
			return i;
	return -1;
}

int RgbtSkewPolytope::FindEdgeWithRightVidAs(int id) {
	for (int i = 0; i < m_cross_edges.size(); i++)
		if (m_cross_edges[i].v2_id == id)
			return i;
	return -1;
}

void RgbtSkewPolytope::SlicePolyhedronForAllFramesAndWrite(string sliced_3d_poly_path, string sliced_polyface_path) {

	//1.slice polytope for each frame
	SlicePolyhedronForAllFrames();

	//2.output obj file
	WriteSlicedPolyhedrons(sliced_3d_poly_path);

	//3.output each face(in the polyhedron)'s 3 vertices' info (3 edges and interpolation weight)
	GetAllSlicedFacesWithTriLine();
	SaveSlicedPolygon2MVC(sliced_polyface_path);
}

void RgbtSkewPolytope::GetAllSlicedFacesWithTriLine() {

	assert(!m_slicevert_edgeAndWt_map.empty());
	assert(!m_sliced_polys.empty());

	m_sliced_faces.clear();
	m_sliced_faces.resize(m_sliced_polys.size());

	for (int i = 0; i < m_sliced_polys.size(); i++) {
		for (int j = 0; j < m_sliced_polys[i].m_simplices.size(); j++) {
			cv::Vec3i simplex = m_sliced_polys[i].m_simplices[j];

			//cout << "simplex: " << simplex << endl;

			SlicedFace sf;
			sf.fid = i;

			int v1_id = simplex[0];
			cv::Vec3d v1 = m_sliced_polys[i].m_vertices[v1_id];

			string fvs = frameVert2String(i, v1);

			int e1_id = m_slicevert_edgeAndWt_map[fvs].first;
			double w1 = m_slicevert_edgeAndWt_map[fvs].second;
			Edge4d e_ab = m_cross_edges[e1_id];

			sf.a_idx = e_ab.v1_id;
			sf.b_idx = e_ab.v2_id;
			sf.w_ab = w1;

			int v2_id = simplex[1];
			cv::Vec3d v2 = m_sliced_polys[i].m_vertices[v2_id];

			fvs = frameVert2String(i, v2);

			int e2_id = m_slicevert_edgeAndWt_map[fvs].first;
			double w2 = m_slicevert_edgeAndWt_map[fvs].second;
			Edge4d e_cd = m_cross_edges[e2_id];

			sf.c_idx = e_cd.v1_id;
			sf.d_idx = e_cd.v2_id;
			sf.w_cd = w2;


			int v3_id = simplex[2];
			cv::Vec3d v3 = m_sliced_polys[i].m_vertices[v3_id];

			fvs = frameVert2String(i, v3);

			int e3_id = m_slicevert_edgeAndWt_map[fvs].first;
			double w3 = m_slicevert_edgeAndWt_map[fvs].second;
			Edge4d e_ef = m_cross_edges[e3_id];

			sf.e_idx = e_ef.v1_id;
			sf.f_idx = e_ef.v2_id;
			sf.w_ef = w3;

			/*
			cout << sf.a_idx << " " << sf.w_ab << " " << sf.b_idx << " " <<
					sf.c_idx << " " << sf.w_cd << " " << sf.d_idx << " " <<
					sf.e_idx << " " << sf.w_ef << " " << sf.f_idx << endl;
			*/

			m_sliced_faces[i].push_back(sf);
		}
	}
}

//build the vertex priority queue based on the distance of a vertex to its opposite edge
//smaller distance with higher priority, it is with less impact when removed
void RgbtSkewPolytope::BuildVertexProqueue() {
	for (int i = 0; i < m_vertices.size(); i++) {

		//vertices in the first and last frams can not be removed
		if (fabs(m_vertices[i][3] - 0.0) < ESP || fabs(m_vertices[i][3] - 1.0) < ESP)
			continue;

		//calculate the distances of all vertices to their opposite edges, and save in the priority queue
		int deg = m_vert_adjverts[i].size();
		if (deg == 2) {
			set<int>::iterator it = m_vert_adjverts[i].begin();
			int uid = *it;  it++;
			int vid = *it;

			cv::Vec4d ui = m_vertices[i] - m_vertices[uid];
			cv::Vec4d uv_normal = cv::normalize(m_vertices[vid] - m_vertices[uid]);
			cv::Vec4d proj = ui.dot(uv_normal) * uv_normal;

			double h = cv::norm(ui - proj);
			m_spv_proque.push(PolyVertex4D(i, h));
		}
	}
}


void RgbtSkewPolytope::SaveSlicedPolygon2MVC(string path) {

	//1.sort the vertices
	set<int> vertices_ids;
	set<pair<int, int> >::iterator it;
	for (int i = 0; i < m_cross_edges.size(); i++) {
		vertices_ids.insert(m_cross_edges[i].v1_id);
		vertices_ids.insert(m_cross_edges[i].v2_id);
	}

	ofstream of(path);
	map<int, int> vert_map;
	int id = 1;

	//2.output vertices
	for (set<int>::iterator it = vertices_ids.begin(); it != vertices_ids.end(); it++) {
		cv::Vec4d vert = m_vertices[*it];
		of << "v\t" << vert[0] << "\t" << vert[1] << "\t" << vert[2] << "\t" << vert[3] << endl;
		vert_map[*it] = id++;
	}

	//3.output edges
	for (int i = 0; i < m_cross_edges.size(); i++) {
		Edge4d& e = m_cross_edges[i];
		int vid_1 = e.v1_id;
		int vid_2 = e.v2_id;

		of << "e\t" << vert_map[vid_1] << "\t" << vert_map[vid_2] << endl;
	}

	//4.output slices
	for (int i = 0; i < m_sliced_faces.size(); i++) {
		for (int j = 0; j < m_sliced_faces[i].size(); j++) {
			SlicedFace& sf = m_sliced_faces[i][j];
			of << "fslice\t" << sf.fid <<
				"\t" << vert_map[sf.a_idx] << "\t" << sf.w_ab << "\t" << vert_map[sf.b_idx] <<
				"\t" << vert_map[sf.c_idx] << "\t" << sf.w_cd << "\t" << vert_map[sf.d_idx] <<
				"\t" << vert_map[sf.e_idx] << "\t" << sf.w_ef << "\t" << vert_map[sf.f_idx] << endl;
		}
	}
	of.close();
}