#pragma once
#include <vector>
#include "libqhullcpp/Qhull.h"
#include "utility.h"
#include <string>

using namespace orgQhull;
using namespace std;

class RgbConvexhull
{
public: 
	vector<cv::Vec3d> m_vertices;
	vector<cv::Vec3i> m_simplices;
	set<pair<int, int> > m_edges;

public:
	RgbConvexhull() {};
	RgbConvexhull(vector<cv::Vec3d> vertices);

	void BuildConvexhull();
	void GetConvexHullEdges();
	void SimplifyConvexHull(int n);

	void CorrectNormal();

	void MergeShortEdge(double len);
	bool IsEnclose(cv::Vec3d point);
	double MinDistance2Point(cv::Vec3d point);
	void WriteConvexhull(string path);

	bool IsInstersection();
	int VertsNumberOnConvexHull();

	void WriteConvexhull_debug(string path);
	void ReadConvexhull_debug(string path);
};