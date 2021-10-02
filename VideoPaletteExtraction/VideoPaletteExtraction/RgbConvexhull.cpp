#include "RgbConvexhull.h"
#include "libqhullcpp/QhullFacetList.h"
#include "libqhullcpp/Qhull.h"

#include <map>
#include <fstream>
#include <set>

using namespace orgQhull;
using namespace std;

RgbConvexhull::RgbConvexhull(vector<cv::Vec3d> vertices) { 
	m_vertices = vertices;

	for (int i = 0; i < m_vertices.size(); i++) 
		if (isnan(m_vertices[i][0]) || isnan(m_vertices[i][1]) || isnan(m_vertices[i][2]))
			throw QhullError();
	
	try {
		BuildConvexhull();
	}
	catch (QhullError e) {
		throw e;
	}
}

void RgbConvexhull::BuildConvexhull() {
	assert(!m_vertices.empty());
	m_simplices.clear();

	Qhull qhull_;
	try {
		qhull_.runQhull3D(m_vertices, "Qt");
		
		QhullFacetList facets = qhull_.facetList();
		for (QhullFacetList::iterator it = facets.begin(); it != facets.end(); ++it){
			if (!(*it).isGood()) continue;
			QhullFacet f = *it;
			QhullVertexSet vSet = f.vertices();

			int fvid[3], k = 0;
			for (QhullVertexSet::iterator vIt = vSet.begin(); vIt != vSet.end(); ++vIt){
				QhullVertex v = *vIt;
				QhullPoint p = v.point();
				fvid[k++] = p.id();
			}
			m_simplices.push_back(cv::Vec3i(fvid[0], fvid[1], fvid[2]));
		}
	}
	catch (QhullError e) {
		throw e;
	}

	GetConvexHullEdges();
}

//reduce the convex hull vertex number
void RgbConvexhull::SimplifyConvexHull(int n) {
	while (m_vertices.size() > n) {
		Edge3d min_dis_edge;
		double min_edge_dis = DBL_MAX;
		for (set<pair<int, int> >::iterator it = m_edges.begin(); it != m_edges.end(); it++) {
			Edge3d curr_edge(m_vertices[it->first], m_vertices[it->second], it->first, it->second);
			if (curr_edge.len < min_edge_dis) {
				min_edge_dis = curr_edge.len;
				min_dis_edge = curr_edge;
			}
		}
		m_vertices[min_dis_edge.v1_id] = (min_dis_edge.v1 + min_dis_edge.v2) * 0.5;
		m_vertices.erase(m_vertices.begin() + min_dis_edge.v2_id);
		
		Qhull qhull_;
		try {
			qhull_.runQhull3D(m_vertices, "Qt");
		}catch (QhullError e) {
			throw e;
		}

		vector<cv::Vec3d> new_vertices;
		for (QhullVertexList::iterator it = qhull_.vertexList().begin(); it != qhull_.vertexList().end(); it++)
			new_vertices.push_back(m_vertices[it->point().id()]);
		
		m_vertices = new_vertices;
		BuildConvexhull();
	}
}

void RgbConvexhull::MergeShortEdge(double len_th) {

	while (1) {

		if (m_vertices.size() <= 4)
			break;
		
		Edge3d min_edge;
		double min_len = DBL_MAX;
		for (set<pair<int, int> >::iterator it = m_edges.begin(); it != m_edges.end(); it++) {
			int uid = it->first, vid = it->second;
			Edge3d e(m_vertices[uid], m_vertices[vid], uid, vid);
			if (e.len < min_len) {
				min_len = e.len;
				min_edge = e;
			}
		}

		if (min_len > len_th)
			break;

		int u_id = min_edge.v1_id, v_id = min_edge.v2_id;
		cv::Vec3d u = min_edge.v1, v = min_edge.v2;
		m_vertices[u_id] = (u + v) * 0.5;
		m_vertices.erase(m_vertices.begin() + v_id);

		Qhull qhull_;
		try {
			qhull_.runQhull3D(m_vertices, "Qt");
		}
		catch (QhullError e) {
			throw e;
		}

		vector<cv::Vec3d> new_vertices;
		for (QhullVertexList::iterator it = qhull_.vertexList().begin(); it != qhull_.vertexList().end(); it++)
			new_vertices.push_back(m_vertices[it->point().id()]);

		m_vertices = new_vertices;
		BuildConvexhull();
	}
}

void RgbConvexhull::GetConvexHullEdges() {
	m_edges.clear();
	for (int j = 0; j < m_simplices.size(); j++) {
		int fv1_id = m_simplices[j][0];
		int fv2_id = m_simplices[j][1];
		int fv3_id = m_simplices[j][2];

		m_edges.insert(make_pair(min(fv1_id, fv2_id), max(fv1_id, fv2_id)));
		m_edges.insert(make_pair(min(fv2_id, fv3_id), max(fv2_id, fv3_id)));
		m_edges.insert(make_pair(min(fv1_id, fv3_id), max(fv1_id, fv3_id)));
	}
}

bool RgbConvexhull::IsEnclose(cv::Vec3d point) {
	int cross_n = 0;
	cv::Vec3d rayDir(1, 0, 0);
	cv::Vec3d interPoint;

	for (int i = 0; i < m_simplices.size(); i++) {
		cv::Vec3i simplex = m_simplices[i];
		cv::Vec3d v0 = m_vertices[simplex[0]];
		cv::Vec3d v1 = m_vertices[simplex[1]];
		cv::Vec3d v2 = m_vertices[simplex[2]];
		cv::Vec3d tri[3] = { v0,v1,v2 };
		
		bool through = rayThroughTriangle(point, rayDir, tri, interPoint);
		if (through) cross_n++;
	}
	return (cross_n % 2);
}


double RgbConvexhull::MinDistance2Point(cv::Vec3d point) {

	bool inside = IsEnclose(point);
	if (inside)
		return 0;

	double distance_min = DBL_MAX;
	for (int i = 0; i < m_simplices.size(); i++) {
		cv::Vec3i simplex = m_simplices[i];
		cv::Vec3d v0 = m_vertices[simplex[0]];
		cv::Vec3d v1 = m_vertices[simplex[1]];
		cv::Vec3d v2 = m_vertices[simplex[2]];

		cv::Vec3d intsec_point;

		cv::Vec3d v01 = v1 - v0;
		cv::Vec3d v02 = v2 - v0;
		cv::Vec3d norm_ = v01.cross(v02);

		//cout << v01 << "\t" << v02 << "\t" << norm_ << endl;

		cv::Vec3d tri[3] = { v0,v1,v2 };

		bool through = rayThroughTriangle(point, -norm_, tri, intsec_point);
		if (through) {
			double dis = cv::norm(point - intsec_point);
			if (dis < distance_min)
				distance_min = dis;
		}
		else {
			cv::Vec3d tri[3] = { v0,v1,v2 };
			through = rayThroughTriangle(point, norm_, tri, intsec_point);
			if (through) {
				double dis = cv::norm(point - intsec_point);
				if (distance_min > dis)
					distance_min = dis;
			}
		}
	}

	if (distance_min == DBL_MAX) 
	{
		for (int i = 0; i < m_vertices.size(); i++) {
			cv::Vec3d v = m_vertices[i];
			double dis = cv::norm(point - v);
			if (distance_min > dis)
				distance_min = dis;
		}
	}
	return distance_min;
}

void RgbConvexhull::WriteConvexhull(string path) {
	ofstream of(path);
	set<int> vertices_ids;
	for (int i = 0; i < m_simplices.size(); i++) {
		cv::Vec3i simplex = m_simplices[i];
		vertices_ids.insert(simplex[0]);
		vertices_ids.insert(simplex[1]);
		vertices_ids.insert(simplex[2]);
	}

	set<int>::iterator it;
	map<int, int> mp;
	int id = 1;

	for (it = vertices_ids.begin(); it != vertices_ids.end(); it++){
		cv::Vec3d vert = m_vertices[*it];
		of << "v " << vert[0] << " " << vert[1] << " " << vert[2] << endl;
		mp[*it] = id++;
	}

	for (int i = 0; i < m_simplices.size(); i++) {
		cv::Vec3i simplex = m_simplices[i];
		int fvid_0 = simplex[0];
		int fvid_1 = simplex[1];
		int fvid_2 = simplex[2];

		of << "f " << mp[fvid_0] << " " << mp[fvid_1] << " " << mp[fvid_2] << endl;
	}
}

void RgbConvexhull::WriteConvexhull_debug(string path) {
	ofstream of(path);
	
	of << m_vertices.size() << endl;
	for (int i = 0; i < m_vertices.size(); i++) {
		cv::Vec3d vert = m_vertices[i];
		of << vert[0] << " " << vert[1] << " " << vert[2] << endl;
	}

	of << m_edges.size() << endl;
	for (set<pair<int, int> >::iterator it = m_edges.begin(); it != m_edges.end(); it++) {
		of << it->first << " " << it->second << endl;
	}
}

void RgbConvexhull::ReadConvexhull_debug(string path) {
	ifstream of(path);
	int vn; of >> vn;
	
	for (int i = 0; i < vn; i++) {
		double a, b, c;
		of >> a >> b >> c;
		cv::Vec3d vert(a, b, c);
		m_vertices.push_back(vert);
	}

	int en; of >> en;
	for (int i = 0; i < en; i++) {
		int a, b;
		of >> a >> b;
		m_edges.insert(make_pair(a, b));
	}
}

void RgbConvexhull::CorrectNormal() {

	for (int i = 0; i < m_simplices.size(); i++) {
		int v1_id = m_simplices[i][0];
		int v2_id = m_simplices[i][1];
		int v3_id = m_simplices[i][2];

		cv::Vec3d v1(m_vertices[v1_id]);
		cv::Vec3d v2(m_vertices[v2_id]);
		cv::Vec3d v3(m_vertices[v3_id]);

		cv::Vec3d v12 = v2 - v1;
		cv::Vec3d v13 = v3 - v1;
		cv::Vec3d cross = v12.cross(v13);
		cv::Vec3d normal = cv::normalize(cross);

		cv::Vec3d center = (v1 + v2 + v3)*0.333;

		cv::Vec3d point = center + normal*0.002;

		if (IsEnclose(point)) {
			int t = m_simplices[i][0];
			m_simplices[i][0] = m_simplices[i][1];
			m_simplices[i][1] = t;
		}
	}
}

bool RgbConvexhull::IsInstersection() {
	
	for (set<pair<int, int> >::iterator it = m_edges.begin(); it != m_edges.end(); it++) {
		int v1_id = it->first;
		int v2_id = it->second;
		cv::Vec3d p1 = m_vertices[v1_id];
		cv::Vec3d p2 = m_vertices[v2_id];
		cv::Vec3d intsec_point;

		for (int i = 0; i < m_simplices.size(); i++) {
			cv::Vec3i simplex = m_simplices[i];
			if (simplex[0] == v1_id || simplex[0] == v2_id ||
				simplex[1] == v1_id || simplex[1] == v2_id ||
				simplex[2] == v1_id || simplex[2] == v2_id)
				continue;

			cv::Vec3d v0 = m_vertices[simplex[0]];
			cv::Vec3d v1 = m_vertices[simplex[1]];
			cv::Vec3d v2 = m_vertices[simplex[2]];
			cv::Vec3d tri[3] = { v0,v1,v2 };

			cv::Vec3d rayDir1 = p2 - p1;
			bool through1 = rayThroughTriangle(p1, rayDir1, tri, intsec_point);

			cv::Vec3d rayDir2 = p1 - p2;
			bool through2 = rayThroughTriangle(p2, rayDir2, tri, intsec_point);

			if (through1 && through2) {			
				return true;
			}
		}
	}
	return false;
}

int RgbConvexhull::VertsNumberOnConvexHull() {
	set<int> vids;
	for (int i = 0; i < m_simplices.size(); i++) {
		cv::Vec3i simplex = m_simplices[i];
		vids.insert(simplex[0]);
		vids.insert(simplex[1]);
		vids.insert(simplex[2]);
	}
	return vids.size();
}