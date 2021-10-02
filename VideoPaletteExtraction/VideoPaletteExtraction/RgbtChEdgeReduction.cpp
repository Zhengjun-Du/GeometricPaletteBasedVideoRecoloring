#include "RgbtChEdgeReduction.h"
#include <iostream>
#include <numeric>
#include <fstream>

RgbtChEdgeReduction::RgbtChEdgeReduction() {
	m_best_cost = DBL_MAX;
}

RgbtChEdgeReduction::RgbtChEdgeReduction(
	int vert_cnt,
	set<pair<int, int> >& topo1,
	set<pair<int, int> >& topo2) { 

	m_vert_cnt = vert_cnt; 
	m_best_cost = DBL_MAX;
	m_topo1 = topo1, m_topo2 = topo2;
}

bool RgbtChEdgeReduction::CheckMappingConstraint(int frame_id, vector<int>& u_edge_ends, vector<int>& v_edge_ends) {
	for (int i = 0; i < u_edge_ends.size(); i++) {
		for (int j = 0; j < v_edge_ends.size(); j++) {
			int vi = u_edge_ends[i], vj = v_edge_ends[j];
			if (vi == vj)
				return true;
			else {
				pair<int, int> e(min(vi, vj), max(vi, vj));
				if (frame_id == 1 && m_topo1.find(e) != m_topo1.end())
					return true;
				if (frame_id == 2 && m_topo2.find(e) != m_topo2.end())
					return true;
			}
		}
	}
	return false;
}

// check the connection constraints
bool RgbtChEdgeReduction::CheckConnectionConstraint(vector<Edge4d>& cross_edges) {

	//the corresponding vertices of cross edges(fram 1 -> frame 2 and frame 2 -> frame 1)
	m_vert_incident_edges_mp1.clear();
	m_vert_incident_edges_mp2.clear();

	//1. check if the cross edges have convered all vertices in adjacent frames' convex hulls
	set<int> cross_edge_vids;
	int u_vid = 0;
	int v_vid = 0;

	for each(Edge4d e in cross_edges) {

		//(u,v) in each cross edge(inter-frame edge)
		u_vid = e.v1_id;
		v_vid = e.v2_id;
		cross_edge_vids.insert(u_vid);
		cross_edge_vids.insert(v_vid);

		int min_vid = min(u_vid, v_vid);
		int max_vid = max(u_vid, v_vid);

		//vertex correspondence(1:1 or 1:n)
		m_vert_incident_edges_mp1[min_vid].push_back(max_vid);
		m_vert_incident_edges_mp2[max_vid].push_back(min_vid);

	}
	//exsit isolated vertices 
	if (cross_edge_vids.size() != m_vert_cnt)
		return false;

	//2. check the edges correspondence(f1->f2 and f2->f1)
	//2.1 check the edges in frame 1
	int corresspont_cnt = 0;

	set<pair<int, int> >::iterator it;
	for (it = m_topo1.begin(); it != m_topo1.end(); it++) {
		u_vid = it->first;
		v_vid = it->second;

		//edge(u_vid,v_vid) in frame 1, u_edge_ends: vertices in frame 2 that connect u_vid
		vector<int>& u_edge_ends = m_vert_incident_edges_mp1[u_vid];

		//edge(u_vid,v_vid) in frame 1, v_edge_ends: vertices in frame 2 that connect v_vid
		vector<int>& v_edge_ends = m_vert_incident_edges_mp1[v_vid];

		//reasonable correspondence：edge to edge, or edge to vertex
		bool ok = CheckMappingConstraint(2, u_edge_ends, v_edge_ends);
		corresspont_cnt += ok;
	}

	if (corresspont_cnt != m_topo1.size())
		return false;

	//2.2 check the edges in frame 2, same with 2.1
	corresspont_cnt = 0;
	for (it = m_topo2.begin(); it != m_topo2.end(); it++) {
		u_vid = it->first;
		v_vid = it->second;

		vector<int>& u_edge_ends = m_vert_incident_edges_mp2[u_vid];
		vector<int>& v_edge_ends = m_vert_incident_edges_mp2[v_vid];

		corresspont_cnt += CheckMappingConstraint(1, u_edge_ends, v_edge_ends);
	}
	if (corresspont_cnt != m_topo2.size())
		return false;

	return true;
}

//check degree constraint:the end vertices' degrees of the cross edge cannot be > = 2 at the same time
bool RgbtChEdgeReduction::CheckDegreeConstraint(vector<Edge4d>& cross_edges) {

	int u_vid, v_vid, min_vid, max_vid;
	for each(Edge4d e in cross_edges) {
		u_vid = e.v1_id;
		v_vid = e.v2_id;

		min_vid = min(u_vid, v_vid);
		max_vid = max(u_vid, v_vid);

		if (m_vert_incident_edges_mp1[min_vid].size() >= 2 &&
			m_vert_incident_edges_mp2[max_vid].size() >= 2) {
			return true;
		}
	}
	return false;
}

double RgbtChEdgeReduction::GetConnectivityCost(vector<Edge4d>& edges) {
	double sum_len = 0;
	for each(Edge4d e in edges)
		sum_len += e.rgb_len;
	return sum_len;
}

void RgbtChEdgeReduction::FindBestConnectivity_(vector<Edge4d> cross_edges) {
	string edges_str = edgeVec2String(cross_edges);

	//if visited, return
	if (m_record[edges_str])
		return;

	//if the connection constraint is not satisfied
	bool connect = CheckConnectionConstraint(cross_edges);
	if (!connect) {
		m_record[edges_str] = DBL_MAX;
		return;
	}

	//calculate the cost，if cost < m_best_cost: update cost
	double cost = GetConnectivityCost(cross_edges);
	m_record[edges_str] = cost;

	//if the degree contraint is satisfied, update
	bool n2n = CheckDegreeConstraint(cross_edges);
	if (m_best_cost > cost && !n2n) {
		m_best_cost = cost;
		m_best_cross_edges = cross_edges;
	}

	//dfs
	for (int i = 0; i < cross_edges.size(); i++) {
		vector<Edge4d> sub_egdes;
		sub_egdes.insert(sub_egdes.end(), cross_edges.begin(), cross_edges.begin() + i);
		sub_egdes.insert(sub_egdes.end(), cross_edges.begin() + i + 1, cross_edges.end());

		FindBestConnectivity_(sub_egdes);
	}
}

vector<Edge4d> RgbtChEdgeReduction::FindBestConnectivity(vector<Edge4d> cross_edges) {
	FindBestConnectivity_(cross_edges);
	m_record.clear();
	return m_best_cross_edges;
}