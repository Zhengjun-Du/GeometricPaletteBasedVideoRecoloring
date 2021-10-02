#pragma once

#include <vector>
#include <set>
#include <map>
#include <utility>
#include "libqhullcpp/Qhull.h"
#include "RgbtConvexhull.h"
#include "RgbConvexhull.h"
#include "RgbtChEdgeReduction.h"

using namespace orgQhull;
using namespace std; 

typedef RgbConvexhull Polygon;

struct PolyVertex4D {
	int vid;	//vertex id
	double len; //distance of v to (u,w) in a triangle uvw
	PolyVertex4D(int id_, double len_) {
		vid = id_;
		len = len_;
	}
	bool operator<(const PolyVertex4D& v)const {
		return len > v.len;
	}
};


class RgbtSkewPolytope{
public:
	vector<cv::Vec4d> m_vertices;
	vector<Edge4d> m_cross_edges;

	//vertex's adjacent vertices
	map<int, set<int> > m_vert_adjverts;

	//sliced 3D polyhedron
	vector<RgbConvexhull> m_sliced_polys;

	//block and its frames
	vector<vector<int> > m_frm_blocks;
	vector<Frame>* m_pframes;

	//vertex priority queue
	priority_queue<PolyVertex4D>  m_spv_proque;

	map<string, pair<int, double> >  m_slicevert_edgeAndWt_map;
	vector< vector<SlicedFace> > m_sliced_faces; 

public:
	RgbtSkewPolytope() {};

	void GetAllSlicedFacesWithTriLine();

	//sice skew polytope
	bool SlicePolyhedronForAllFrames();
	RgbConvexhull SlicePolyhedronForFrame(int fid, bool record_slice_info = false);
	void SlicePolyhedronForAllFramesAndWrite(string sliced_3d_poly_path, string sliced_polyface_path);

	//output polyhedron
	void WriteSlicedPolyhedrons(string path);

	//remove vertex
	void RemoveVertex(int vid);
	void TryRemoveVertex(int vid);

	//buid the vertex priority queue
	void BuildVertexProqueue();

	int FindEdgeWithLeftVidAs(int id);
	int FindEdgeWithRightVidAs(int id);

	void SaveSlicedPolygon2MVC(string path);
};