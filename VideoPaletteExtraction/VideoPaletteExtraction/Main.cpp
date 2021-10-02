#include "VideoPalette.h"
#include <iostream>
#include <vector>
#include <fstream>
using namespace std;

void GenerateVideoPalette1(string para_path) {
	int frm_cnt = 50;
	string base_dir = "../../data/season/";
	SysParameter::SetFrameCnt(frm_cnt);
	SysParameter::SetSampFrameCnt(frm_cnt / 4);

	VideoPalette VP_Extractor;
	VP_Extractor.ReadVideoAndFrameRgbConvexhulls(base_dir);
	VP_Extractor.BuildAndMergeFrameBlocks();
	VP_Extractor.SimplifyVideoPalette();
	VP_Extractor.RefineVideoPalette();

	string sliced_3d_poly_path = base_dir + "frm_palette_%05d.obj";
	string sliced_polyface_path = base_dir + "z_polyhedron_slices.obj";
	VP_Extractor.WriteEachFramePalette(sliced_3d_poly_path, sliced_polyface_path);

	cout << "generate video file and polyface..." << endl;
	string script_path = "./scripts/polysliceobj_to_polyface.py";
	system(("python " + script_path + " " + base_dir + " season.polyface").c_str());
	cout << "finish to generate video file and polyface" << endl;
}

int main(int argc, char *argv[]) {
	string para_path = "./parameter.yaml";
	SysParameter::SetSysParameters(para_path);
	GenerateVideoPalette1(para_path);
	return 0;
}