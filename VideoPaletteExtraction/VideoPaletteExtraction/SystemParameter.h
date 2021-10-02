#pragma once

#include <string>
#include <opencv2/core.hpp>
#include <fstream>
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

class SysParameter {
private:
	static int frmCnt; 
	static int sampFrmCnt;
	static int sampFrmPixCnt;

	static double frmRefineRlossWt;
	static double frmRefineClossWt;

	static double blockAlpha;
	static double blockBeta;
	static double blockRlossWt;
	static double blockClossWt;
	static double blockSlossWt;
	static double blockTh;

	static double simplifyRlossWt;
	static double simplifyClossWt;
	static double simplifySlossWt;
	static double simplifyTh;

public:
	static void SetSysParameters(string path) {

		cv::FileStorage file_settings(path, cv::FileStorage::READ);
		sampFrmPixCnt = file_settings["frm_samp_pix_cnt"];
		frmRefineRlossWt = file_settings["rloss_factor"];
		frmRefineClossWt = file_settings["closs_factor"];

		blockAlpha = file_settings["blk_alpha"];
		blockBeta = file_settings["blk_beta"];
		blockRlossWt = file_settings["rloss_factor"];
		blockClossWt = file_settings["closs_factor"];
		blockSlossWt = file_settings["sloss_factor"];

		simplifyRlossWt = file_settings["rloss_factor"];
		simplifyClossWt = file_settings["closs_factor"];
		simplifySlossWt = file_settings["sloss_factor"];

		blockTh = file_settings["block_th"];
		simplifyTh = file_settings["simplify_th"];
	}

	static void SetSysParameters(
		string base_dir_,
		int frm_cnt,
		int sam_frm_cnt,
		int sam_pix_cnt,
		float b_alpha, 
		float b_beta, 
		float r, 
		float c, 
		float s,
		float block_th,
		float simplify_th) {

		frmCnt = frm_cnt;
		sampFrmCnt = sam_frm_cnt;
		sampFrmPixCnt = sam_pix_cnt;

		frmRefineRlossWt = r;
		frmRefineClossWt = c;

		blockAlpha = b_alpha;
		blockBeta = b_beta;
		blockRlossWt = r;
		blockClossWt = c;
		blockSlossWt = s;				

		simplifyRlossWt = r;
		simplifyClossWt = c;
		simplifySlossWt = s;

		blockTh = block_th;	
		simplifyTh = simplify_th;
	}

	static void SetFrameCnt(int frm_cnt) { frmCnt = frm_cnt; }
	static int FrameCnt() { return frmCnt; }
	static void SetSampFrameCnt(int n) { sampFrmCnt = n; }
	static int SampFrameCnt() { return sampFrmCnt; }
	static int SampFrmPixCnt() { return sampFrmPixCnt; }
	static double FrmRefineRlossWt() { return frmRefineRlossWt; }
	static double FrmRefineClossWt() { return frmRefineClossWt; }

	static double BlockAlpha() { return blockAlpha; }
	static double BlockBeta() { return blockBeta; }
	static double BlockRlossWt() { return blockRlossWt; }
	static double BlockClossWt() { return blockClossWt; }
	static double BlockSlossWt() { return blockSlossWt; }
	static double BlockTh() { return blockTh; }

	static double SimplifyRlossWt() { return simplifyRlossWt; }
	static double SimplifyClossWt() { return simplifyClossWt; }
	static double SimplifySlossWt() { return simplifySlossWt; }
	static double SimplifyTh() { return simplifyTh; }
	static void SetBlockTh(int th) { blockTh = th; }
};