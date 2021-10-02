#pragma once

#include <vector>
#include "libqhullcpp/Qhull.h"
#include "utility.h"
#include "RgbConvexhull.h"
#include "Frame.h"

using namespace orgQhull;
using namespace std; 

enum ReMoveStatus { PremoveEdgeSuccess, PremoveEdgeFail, FinalRemoveEdgeSuccess, FinalRemoveEdgeFail };

class RgbtConvexhull
{
public:
	vector<cv::Vec4d> m_vertices;
	vector<cv::Vec4i> m_simplices;
	vector<Edge4d> m_cross_edges;

public:
	RgbtConvexhull(){ }
	RgbtConvexhull(vector<cv::Vec4d>& vertices);
	RgbtConvexhull(Frame& f1, Frame& f2);

	void BuildRgbtConvexhull();
	void GetCrossEdges();
	void ForceEdgeConnect(RgbConvexhull& ch1, RgbConvexhull& ch2);

	ReMoveStatus PreRemoveSomeCrossEdges(RgbConvexhull& ch1, RgbConvexhull& ch2);
	ReMoveStatus RemoveRedundantCrossEdges(RgbConvexhull& ch1, RgbConvexhull& ch2);
};