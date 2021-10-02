#pragma once
#include "utility.h"
#include "Frame.h"
#include <map>

using namespace std;

//remove redudant edges in RGBT convex hull
class RgbtChEdgeReduction{
public: 
	vector<Edge4d> m_best_cross_edges;		//final remaining cross edges(inter-frame edges)
	double m_best_cost;						//lowest cost

private:
	map<string, double> m_record;			//for acceleration, record the visit info
	set<pair<int, int> > m_topo1, m_topo2;	//intra-frame edges in fram 1 and frame 2
	int m_vert_cnt;							//vertices totoal number
	map<int, vector<int> > m_vert_incident_edges_mp1;	//vertex's incident edges in frame 1
	map<int, vector<int> > m_vert_incident_edges_mp2;	//vertex's incident edges in frame 2

public:
	RgbtChEdgeReduction(
		int vert_cnt,
		set<pair<int, int> >& topo1, 
		set<pair<int, int> >& topo2
	);

	RgbtChEdgeReduction();

	bool CheckConnectionConstraint(vector<Edge4d>& cross_edges);
	double GetConnectivityCost(vector<Edge4d>& edges);
	vector<Edge4d> FindBestConnectivity(vector<Edge4d> cross_edges);

private:
	void FindBestConnectivity_(vector<Edge4d> cross_edges);
	bool CheckDegreeConstraint(vector<Edge4d>& cross_edges);
	bool CheckMappingConstraint(int frame_id, vector<int>& u_edge_ends, vector<int>& v_edge_ends);
};