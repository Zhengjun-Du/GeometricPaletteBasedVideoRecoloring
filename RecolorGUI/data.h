#ifndef DATA_H
#define DATA_H

#include <QString>
#include <QObject>
#include <vector>
#include <QThread>
#include "utility.h"
#include "vec3.h"
#include <vector>
#include <QTime>
using namespace std;

#define RGBXY	1
#define LBC		2
#define M_PI 3.14159265358979323846
#define ESP 1e-4

struct comp {
	bool operator()(const vec3& v1, const vec3& v2)const {
		if (fabs(v1[0] - v2[0]) > ESP)
			return v1[0] < v2[0];
		else if (fabs(v1[1] - v2[1]) > ESP)
			return v1[1] < v2[1];
		else if (fabs(v1[2] - v2[2]) > ESP)
			return v1[2] < v2[2];
		return false;
	}
};

class Data : public QObject
{
	Q_OBJECT

public:
	Data();
	void openVideo(QString fileName);
	void openPoly(QString fileName, bool isPolyface);
	void resize();
	void recalcPaletteHeight();

	double *getVideo(bool isAfter = true) const { return isAfter ? recolored_video : original_video; }
	VideoPalette getCurVideoPalette() const { return current_palette; }
	VideoPalette getOriVideoPalette() const { return original_palette; }

	vector<float> ComputeSingleVertexMVCWeights(int time, int vid, const vec3 &x);
	vector<float> ComputeSingleVertexMVCWeightsForceInside(int time, int vid, const vec3 &x);
    void ComputeMVCWeights();
    void ComputeMVCWeightsWithoutColorStop();
    void ComputeMVCWeightsWithColorStop();
    void ComputeRgbxyWeights();
    void ComputeRgbxyOrLbcWeights(int flag);

	vec3 RecolorSinglePixel(int time, int vid, const vec3 &x);
	void Recolor(int t);
    void UpdateSlicePoly(int t);

    Polyhedron GetSlicePolyhedronOfFrme(int frm_id, bool original);
	void setThreshold(double value);
	void setVertex(int id, QColor c);
	void resetVertex(int id);
	void resetAllVertex();
	const double *getCurVideoPalette_palette_position() const;

	void setCurVideoPalette_palette_position(int i, double y);

	void ExportWeights(string path);
	void ImportWeights(string path);
	void ExportChangedPalette(string path);
	void ImportChangedPalette(string path);
	void ExportRecoloredVideo(string path);
	void ExportOriginalVideo(string path);
    void ExportTikz(QString path, double preview);

	vector<vec3> ReadFramePalette(int fid);

	int getFrmCnt() const { return frame_cnt; }
	int getFrmWidth() const { return frame_width; }
	int getFrmHeight() const { return frame_height; }
	int getFrmDepth() const { return frame_depth; }
	int getTime() const { return time; }

    const vector<vector<map<double, vec3> > > &getColorStop() const;
    void setColorStop(int x, int y, double w, vec3 c);

    const Polyhedron& getSlice_polys(int i) const;

public slots:
signals:
    void updated();

private:
    double* original_video = nullptr;
	double* recolored_video = nullptr;

	VideoPalette original_palette;
	VideoPalette current_palette;

	vector<vector<vector<float>>> mixing_weights;
	vector<double> vertex_palette_position;
	vector<vector<TripleSegment>> faces;
	vector<Polyhedron> slice_polys;

    vector<vector<map<double, vec3>>> colorstop;

	vector<map<vec3, int, comp> > frm_vert_2_palette_vert_maps;

	double threshold = 0.2;
	bool is_weights_calculated;

	string base_dir = "";

	vector<QString> debug;
	int frame_cnt = 0;
	int frame_width = 0;
	int frame_height = 0;
	int frame_depth = 0;
	int time = 0;
	int palette_vcnt = 0;

    bool hasColorStop;

    QTime timeOpen;
    bool isEdited = false;
    QTime timeFirstEdit;
    QTime timeLastEdit;
    QTime timeSave;

};

#endif // DATA_H
