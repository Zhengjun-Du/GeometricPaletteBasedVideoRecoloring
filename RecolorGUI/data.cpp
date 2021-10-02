#include "data.h"
#include "utility.h"
#include <QFile>
#include <QDebug>
#include <algorithm>
#include <cmath>
#include <QProgressDialog>
#include <QThread>
#include <QMessagebox>
#include <QTime>
#include <omp.h>
#include <map>
#include "my_util.h"
#include <fstream>
#include <string>
#include <QDir>
#include <QRandomGenerator>
#include <QTime>
using namespace std;


const double* Data::getCurVideoPalette_palette_position() const {
	return vertex_palette_position.empty() ? nullptr : vertex_palette_position.data();
}

void Data::setCurVideoPalette_palette_position(int i, double y){
	if (vertex_palette_position.empty()) return;
	vertex_palette_position[i] = y;
	emit updated();
}

Data::Data(){
}

void Data::openVideo(QString fileName){
	string path = fileName.toStdString();
	string s1 = path.substr(0, path.find_last_of("/"));
	base_dir = s1.substr(0, s1.find_last_of("/"));

	QFile file(fileName);
	file.open(QIODevice::ReadOnly);
	file.read(reinterpret_cast<char *>(&frame_cnt), 4);
	file.read(reinterpret_cast<char *>(&frame_height), 4);
	file.read(reinterpret_cast<char *>(&frame_width), 4);
	file.read(reinterpret_cast<char *>(&frame_depth), 4);
	time = 0;

	long long totalSize = frame_cnt * frame_width * frame_height * frame_depth;
	if (recolored_video != nullptr) {
		delete[] recolored_video;
		recolored_video = nullptr;
	}
	if (original_video != nullptr) {
		delete[] original_video;
		original_video = nullptr;
	}

	qDebug() << frame_cnt << frame_width << frame_height << frame_depth << "total" << totalSize;
	qDebug() << totalSize * static_cast<long long>(sizeof(double));

	recolored_video = new double[totalSize];
	original_video = new double[totalSize];

	file.read(reinterpret_cast<char *>(recolored_video), totalSize * static_cast<long long>(sizeof(double)));
	memcpy(original_video, recolored_video, sizeof(double)*totalSize);

	file.close();

	emit updated();
}

void Data::openPoly(QString fileName, bool isPolyface){
	int vert_cnt = 0, edge_cnt = 0, face_cnt = 0;
	QFile file(fileName);

	file.open(QIODevice::ReadOnly);
	file.read(reinterpret_cast<char *>(&frame_cnt), 4);
	file.read(reinterpret_cast<char *>(&vert_cnt), 4);
	file.read(reinterpret_cast<char *>(&edge_cnt), 4);
	if (isPolyface) file.read(reinterpret_cast<char *>(&face_cnt), 4);

	debug.resize(vert_cnt);
	palette_vcnt = vert_cnt;

	//1. read palette vertices
	vector<double> palette_vertices;
	palette_vertices.resize(vert_cnt * 4);
	vertex_palette_position.resize(vert_cnt);

	file.read(reinterpret_cast<char *>(palette_vertices.data()), sizeof(double) * vert_cnt * 4);
	original_palette.clear();
	for (int i = 0; i < vert_cnt; i++) {
		float r = palette_vertices[i * 4 + 0];
		float g = palette_vertices[i * 4 + 1];
		float b = palette_vertices[i * 4 + 2];
		float t = palette_vertices[i * 4 + 3];
		original_palette.vertices.push_back(vec4(r, g, b, t));
	}

	//2. read palette edges
	vector<int> palette_edges;
	palette_edges.resize(edge_cnt * 2);
	file.read(reinterpret_cast<char *>(palette_edges.data()), sizeof(int)*edge_cnt * 2);

	for (int i = 0; i < edge_cnt; i++) {
		int uid = palette_edges[i * 2 + 0];
		int vid = palette_edges[i * 2 + 1];
		original_palette.edges.push_back(int2(uid, vid));
	}
	current_palette = original_palette;

	//3. read all slices
	faces.clear(); //if not clear, may be remaining previous values
	faces.resize(frame_cnt);

	for (int i = 0; i < face_cnt; i++) {
		int fid, n_a, n_b, n_c, n_d, n_e, n_f;
		file.read(reinterpret_cast<char *>(&fid), sizeof(int));
		file.read(reinterpret_cast<char *>(&n_a), sizeof(int));
		file.read(reinterpret_cast<char *>(&n_b), sizeof(int));
		file.read(reinterpret_cast<char *>(&n_c), sizeof(int));
		file.read(reinterpret_cast<char *>(&n_d), sizeof(int));
		file.read(reinterpret_cast<char *>(&n_e), sizeof(int));
		file.read(reinterpret_cast<char *>(&n_f), sizeof(int));

		qDebug() << "FACE" << i << "|" << fid << n_a << n_b << n_c << n_d << n_e << n_f;
		TripleSegment ts(n_a, n_b, n_c, n_d, n_e, n_f);
		faces[fid].push_back(ts);
	}

	for (int f = 0; f < frame_cnt; f++) {
		for (int i = 0; i < faces[f].size(); i++) {
			double w1, w2, w3;

			file.read(reinterpret_cast<char *>(&w1), sizeof(double));
			file.read(reinterpret_cast<char *>(&w2), sizeof(double));
			file.read(reinterpret_cast<char *>(&w3), sizeof(double));

			qDebug() << "FACE_alpha" << w1 << w2 << w3;

			faces[f][i].w_ab = w1;
			faces[f][i].w_cd = w2;
			faces[f][i].w_ef = w3;
		}
	}

	slice_polys.clear();
	for (int i = 0; i < frame_cnt; i++) {
        slice_polys.push_back(GetSlicePolyhedronOfFrme(i, true));
	}

    colorstop.clear();
    colorstop.resize(vert_cnt);
    for(int i = 0; i < vert_cnt; i++)
        colorstop[i].resize(vert_cnt);

    /*
    for(int i = 0; i < vert_cnt; i++)
        for(int j = 0; j < vert_cnt; j++)
        {
            colorstop[i][j][0.4] = vec3(0x66/255., 0xcc/255., 0xff/255.);
        }
    */
    hasColorStop = false;

	file.close();
	emit updated();

    timeOpen = QTime::currentTime();
    isEdited = false;
}

void Data::resize(){
	mixing_weights.clear();
	mixing_weights.resize(frame_cnt);
	for (int i = 0; i < frame_cnt; i++)
		mixing_weights[i].resize(frame_width*frame_height);

	slice_polys.clear();
	is_weights_calculated = false;
}

void Data::setThreshold(double value){
	threshold = value;
	recalcPaletteHeight();
	emit updated();
}

static QPair<int, int> getfa(vector<vector<QPair<int, int>>> &fa, const QPair<int, int>& x){
	QPair<int, int> &val = fa[x.first][x.second];
	if (val == x) return x;
	return val = getfa(fa, val);
}

static int getfa(vector<int> &fa, const int x){
	int &val = fa[x];
	if (val == x) return x;
	return val = getfa(fa, val);
}

inline double angle(const vec3 & u1, const vec3 & u2) {
	float u_norm = (u1 - u2).norm();
    return 2.0 * myasin(u_norm / 2.0);
}

void Data::setVertex(int id, QColor c) {
	current_palette.vertices[id].v[0] = qRed(c.rgb()) / 255.0;
	current_palette.vertices[id].v[1] = qGreen(c.rgb()) / 255.0;
	current_palette.vertices[id].v[2] = qBlue(c.rgb()) / 255.0;

    if(isEdited == false)
    {
        isEdited = true;
        timeFirstEdit = QTime::currentTime();
    }
    timeLastEdit = QTime::currentTime();
}

vector<float> Data::ComputeSingleVertexMVCWeights(int time, int vid, const vec3 &o) {

	const double eps = 1e-6;

	const Polyhedron &poly = slice_polys[time];
	int vcnt = poly.vertices.size();

	vector<float> d(vcnt);
	vector<vec3> u(vcnt);
	vector<float> weights(vcnt);
	vector<float> w_weights(vcnt);

	for (int v = 0; v < vcnt; v++) {
		d[v] = 0;
		u[v] = vec3(0, 0, 0);
		weights[v] = 0;
	}

	for (int v = 0; v < vcnt; v++) {
		d[v] = (o - poly.vertices[v]).norm();
		if (d[v] < eps) {
			weights[v] = 1.0;
			return weights;
		}
		u[v] = (poly.vertices[v] - o) / d[v];
	}

	float sumWeights = 0.0f;
	vector<float> w(vcnt);

	int fcnt = poly.faces.size();
	for (int i = 0; i < fcnt; i++) {
		int vids[3] = { poly.faces[i].x, poly.faces[i].y, poly.faces[i].z };
		float l[3] = { (u[vids[1]] - u[vids[2]]).norm(), (u[vids[2]] - u[vids[0]]).norm(), (u[vids[0]] - u[vids[1]]).norm() };
        float theta[3] = { 2.0f * myasin(l[0] / 2.0f),2.0f * myasin(l[1] / 2.0f),2.0f * myasin(l[2] / 2.0f) };
		float sintheta[3] = { sin(theta[0]),sin(theta[1]),sin(theta[2]) };

		float h = (theta[0] + theta[1] + theta[2]) / 2.0;
		float sinh = sin(h);

		if (fabs(M_PI - h) < eps * 1000) {
			// x lies inside the triangle t , use 2d barycentric coordinates :
			float w[3];
			for (int i = 0; i < 3; ++i) {
				w[i] = sintheta[i] * d[vids[(i + 2) % 3]] * d[vids[(i + 1) % 3]];
			}
			sumWeights = w[0] + w[1] + w[2];

			weights[vids[0]] = w[0] / sumWeights;
			weights[vids[1]] = w[1] / sumWeights;
			weights[vids[2]] = w[2] / sumWeights;

			return weights;
		}

		float det = dot(u[vids[0]], cross(u[vids[1]], u[vids[2]]));
		float sign_det = signbit(det) ? -1.0f : 1.0f;

		float s[3], c[3];
		for (int i = 0; i < 3; ++i) {
			c[i] = 2 * sinh * sin(h - theta[i]) / (sintheta[(i + 1) % 3] * sintheta[(i + 2) % 3]) - 1;
            s[i] = sign_det * mysqrt(1 - c[i] * c[i]);
		}

		if (fabs(s[0]) > eps && fabs(s[1]) > eps && fabs(s[2]) > eps)
		{
			for (int i = 0; i < 3; ++i) {
				float w_numerator = theta[i] - c[(i + 1) % 3] * theta[(i + 2) % 3] - c[(i + 2) % 3] * theta[(i + 1) % 3];
				float w_denominator = d[vids[i]] * sintheta[(i + 1) % 3] * s[(i + 2) % 3];
				float w_i = w_numerator / w_denominator;
				sumWeights += w_i;
				w_weights[vids[i]] += w_i;
			}
		}
	}

	for (int v = 0; v < vcnt; ++v)
		weights[v] = w_weights[v] / sumWeights;

	return weights;
}

void Data::ComputeMVCWeights()
{
    if(hasColorStop)
    {
        ComputeMVCWeightsWithColorStop();
    }
    else
    {
        ComputeMVCWeightsWithoutColorStop();
    }
}

void Data::ComputeMVCWeightsWithoutColorStop() {
	/*
	if (is_weights_calculated) {
		QMessageBox::information(NULL, "Info", "MVC finished", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
		return;
	}
	*/

	#pragma omp parallel for num_threads(32)
	for (int t = 0; t < frame_cnt; t++) {
        // if(t != 119) continue;

		for (int i = 0; i < frame_width * frame_height * frame_depth; i += frame_depth) {
			vec3 x(
				original_video[t * frame_width * frame_height * frame_depth + i + 0],
				original_video[t * frame_width * frame_height * frame_depth + i + 1],
				original_video[t * frame_width * frame_height * frame_depth + i + 2]
			);

            // int _r = i / frame_depth / frame_width;
            // int _c = (i / frame_depth) % frame_width;

            // if(_r != 139) continue;
            // if(_c != 229 && _c !=230) continue;

			int vid = i / frame_depth;
			vector<float> weights = ComputeSingleVertexMVCWeights(t, vid, x);

            // qDebug() << _r << _c << weights;

			for (int j = 0; j < weights.size(); j++) {
				if (weights[j] < 0 || weights[j] > 1) {
					weights = ComputeSingleVertexMVCWeightsForceInside(t, vid, x);

                    // qDebug() << _r << _c << "ForceInside" << weights;
					break;
				}
			}

			Polyhedron& poly = slice_polys[t];
			int vcnt = poly.vertices.size();
			mixing_weights[t][vid] = vector<float>(palette_vcnt, 0);

			for (int v = 0; v < vcnt; ++v) {
				//an edge (v1_id, v2_id) corresponding to a vertex
				int v1_id = poly.correspondences[v][0];
				int v2_id = poly.correspondences[v][1];
				float w1 = poly.correspondences[v][2];
				float w2 = 1 - w1;

				//clamp
				if (weights[v] > 1) weights[v] = 1;
				if (weights[v] < 0) weights[v] = 0;

				//weight multiplication
				mixing_weights[t][vid][v1_id] += weights[v] * w1;
				mixing_weights[t][vid][v2_id] += weights[v] * w2;
			}
		}
	}
	is_weights_calculated = true;
	QMessageBox::information(NULL, "Info", "MVC finished", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
}

void Data::ComputeMVCWeightsWithColorStop() {
    /*
    if (is_weights_calculated) {
        QMessageBox::information(NULL, "Info", "MVC finished", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        return;
    }
    */

    #pragma omp parallel for num_threads(32)
    for (int t = 0; t < frame_cnt; t++) {
        for (int i = 0; i < frame_width * frame_height * frame_depth; i += frame_depth) {
            vec3 x(
                original_video[t * frame_width * frame_height * frame_depth + i + 0],
                original_video[t * frame_width * frame_height * frame_depth + i + 1],
                original_video[t * frame_width * frame_height * frame_depth + i + 2]
            );

            int vid = i / frame_depth;
            vector<float> weights = ComputeSingleVertexMVCWeights(t, vid, x);

            for (int j = 0; j < weights.size(); j++) {
                if (weights[j] < 0 || weights[j] > 1) {
                    weights = ComputeSingleVertexMVCWeightsForceInside(t, vid, x);
                    break;
                }
            }

            Polyhedron& poly = slice_polys[t];
            int vcnt = poly.vertices.size();
            mixing_weights[t][vid] = vector<float>(vcnt, 0);

            for (int v = 0; v < vcnt; ++v) {
                if (weights[v] > 1) weights[v] = 1;
                if (weights[v] < 0) weights[v] = 0;
                mixing_weights[t][vid][v] = weights[v];
            }
        }
    }
    is_weights_calculated = true;
    QMessageBox::information(NULL, "Info", "MVC finished", QMessageBox::Yes, QMessageBox::Yes);
}

//depressed
void Data::ComputeRgbxyWeights() {
    if(hasColorStop)
    {
        QMessageBox::information(NULL, "Failed", "Not Supported When Color Stop(s) Exist(s)!", QMessageBox::Yes, QMessageBox::Yes);
        return;
    }
	/*
	#pragma omp parallel for num_threads(32)
	for (int t = 0; t < frame_cnt; t++) {
		ofstream of_data("F:/2020/projects/video_palette/RecolorGUI/rgbxy/raw_data/RGBXY_data_" + to_string(t) + ".txt");
		for (int r = 0; r < frame_height; r++) {
			for (int c = 0; c < frame_width; c++) {
				int vid = t * frame_width * frame_height + r*frame_width + c;
				vector<float> rgbxy(5);
				rgbxy[0] = original_video[vid * frame_depth + 0];
				rgbxy[1] = original_video[vid * frame_depth + 1];
				rgbxy[2] = original_video[vid * frame_depth + 2];
				rgbxy[3] = r*1.0 / frame_height;
				rgbxy[4] = c*1.0 / frame_width;
				of_data << rgbxy[0] << " " << rgbxy[1] << " " << rgbxy[2] << " " << rgbxy[3] << " " << rgbxy[4] << endl;
			}
		}
		of_data.close();
		
		Polyhedron& sub_palette = slice_polys[t];
		int vcnt = sub_palette.vertices.size();
		
		ofstream of_palette("./rgbxy/raw_data/RGB_palette_" + to_string(t) + ".txt");
		for (int v = 0; v < vcnt; ++v) {
			float r = sub_palette.vertices[v][0];
			float g = sub_palette.vertices[v][1];
			float b = sub_palette.vertices[v][2];
			vector<float> rgb = { r,g,b };
			of_palette << r << " " << g << " " << b << endl;
		}
		of_palette.close();

		system("conda activate py27");
		string s = "python F:/2020/projects/video_palette/RecolorGUI/rgbxy/scripts/RGBXY_weights.py " + to_string(t);
		system(s.c_str());
		
		ifstream ifs(("./rgbxy/weights/weights_" + to_string(t) + ".txt").c_str());

		float w;
		for (int i = 0; i < frame_height * frame_width; i++) {
			mixing_weights[t][i] = vector<float>(palette_vcnt, 0);
			for (int j = 0; j < vcnt; j++) {
				ifs >> w;
				int v1_id = sub_palette.correspondences[j][0];
				int v2_id = sub_palette.correspondences[j][1];
				float w1 = sub_palette.correspondences[j][2];
				float w2 = 1 - w1;
				float v1_w = w * w1;
				float v2_w = w * w2;
				mixing_weights[t][i][v1_id] = v1_w;
				mixing_weights[t][i][v2_id] = v2_w;
			}
		}
	}
	is_weights_calculated = true;
	QMessageBox::information(NULL, "Info", "weights finished", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
	*/
}

void Data::ComputeRgbxyOrLbcWeights(int flag) {
    if(hasColorStop)
    {
        QMessageBox::information(NULL, "Failed", "Not Supported When Color Stop(s) Exist(s)!", QMessageBox::Yes, QMessageBox::Yes);
        return;
    }

	#pragma omp parallel for num_threads(32)
	for (int t = 0; t < frame_cnt; t++) {
	
		vector<vec3> frm_verts = ReadFramePalette(t);

		Polyhedron& sub_palette = slice_polys[t];
		int vcnt = sub_palette.vertices.size();

		string weights_path = "";
		if (RGBXY == flag)
			weights_path = base_dir + "/rgbxy/weights/rgbxy_weights_" + to_string(t) + ".bin";
		else if(LBC == flag)
			weights_path = base_dir + "/lbc/lbc_weights/lbc_final/lbc_final_weights_" + to_string(t) + ".bin";

		ifstream ifs(weights_path.c_str(), ios::binary);

		float w;
		for (int i = 0; i < frame_height * frame_width; i++) {
			mixing_weights[t][i] = vector<float>(palette_vcnt, 0);
			float ws[20] = { 0 };
			ifs.read(reinterpret_cast<char*>(ws), sizeof(float) * vcnt);
			for (int j = 0; j < vcnt; j++) {
				w = ws[j];	//当前像素关于当前凸包顶点的权重

				//由于GetSlicePolyhedronOfFrme函数中对顶点重新进行编号，因此编号可能跟frm_palette.obj点的顺序不一致
				//因此用map:frm_vert_2_palette_vert_maps 预存了每一帧凸包点对应的palette中的两个顶点
				int k = frm_vert_2_palette_vert_maps[t][frm_verts[j]] - 1;  //frm_verts[j]对应的sub_palette中的顶点编号
				int v1_id = sub_palette.correspondences[k][0];
				int v2_id = sub_palette.correspondences[k][1];
				
				float w1 = sub_palette.correspondences[k][2];	//对应的权重 w1
				float w2 = 1 - w1;								//对应的权重 w2

				mixing_weights[t][i][v1_id] = w * w1; //当前像素关于palette顶点的权重
				mixing_weights[t][i][v2_id] = w * w2; //当前像素关于palette顶点的权重
			}
		}
	}

	is_weights_calculated = true;
	QMessageBox::information(NULL, "Info", "compute weights finished", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
}


vec3 Data::RecolorSinglePixel(int time, int vid, const vec3 &x) {
    vec3 ans(0.f, 0.f, 0.f);

    if(!hasColorStop)
    {
        for (int i = 0; i < mixing_weights[time][vid].size(); i++) {
            float w = mixing_weights[time][vid][i];
            vec3 v(current_palette.vertices[i][0], current_palette.vertices[i][1], current_palette.vertices[i][2]);
            ans = ans + v * w;

            for (int j = 0; j < 3; j++) {
                if (ans[j] < 0) ans[j] = 0;
                else if (ans[j] > 1) ans[j] = 1;
            }
        }
    }
    else
    {
        const Polyhedron& poly = slice_polys[time];
        for (int i = 0; i < mixing_weights[time][vid].size(); i++) {
            //当前顶点RGB多面体上v对应的RGBT上一条边(v1_id, v2_id)
            ans = ans + poly.current_vertices[i] * mixing_weights[time][vid][i];
        }
    }
    return ans;
}

void Data::Recolor(int t) {

	if (!is_weights_calculated || mixing_weights.empty()) {
		emit updated();
		return;
	}

    UpdateSlicePoly(t);

	#pragma omp parallel for num_threads(32)
	for (int i = 0; i < frame_width * frame_height * frame_depth; i += frame_depth)
	{
		vec3 x(
			original_video[t * frame_width * frame_height * frame_depth + i + 0],
			original_video[t * frame_width * frame_height * frame_depth + i + 1],
			original_video[t * frame_width * frame_height * frame_depth + i + 2]
		);

		vec3 y = RecolorSinglePixel(t, i / frame_depth, x);

		recolored_video[t * frame_width * frame_height * frame_depth + i + 0] = y[0];
		recolored_video[t * frame_width * frame_height * frame_depth + i + 1] = y[1];
		recolored_video[t * frame_width * frame_height * frame_depth + i + 2] = y[2];
	}
    emit updated();
}

void Data::UpdateSlicePoly(int t)
{
    Polyhedron& poly = slice_polys[t];
    for(int i = 0; i < (int)poly.vertices.size(); i++)
    {
        int v1_id = poly.correspondences[i][0];
        int v2_id = poly.correspondences[i][1];
        float w1 = 1 - poly.correspondences[i][2];

        const vector<vec4> &cur = current_palette.vertices;

        if(cur[v1_id][3] > cur[v2_id][3])
        {
            std::swap(v1_id, v2_id);
            w1 = 1 - w1;
        }

        colorstop[v1_id][v2_id][0] = vec3(cur[v1_id][0], cur[v1_id][1], cur[v1_id][2]);
        colorstop[v1_id][v2_id][1] = vec3(cur[v2_id][0], cur[v2_id][1], cur[v2_id][2]);

        map<double, vec3>::const_iterator dr = colorstop[v1_id][v2_id].lower_bound(w1);
        map<double, vec3>::const_iterator dl = (dr == colorstop[v1_id][v2_id].begin() ? dr : next(dr, -1));
        if(dr == colorstop[v1_id][v2_id].end()) dl = next(dl, -1), dr = next(dr, -1);

        qDebug() << "Data::UpdateSlicePoly" << w1 << dl->first << dl->second << "|" << dr->first << dr->second;

        if(fabs(dr->first - dl->first) > 1e-6)
        {
            const vec3 c = (dr->second - dl->second) * (w1 - dl->first) / (dr->first - dl->first) + dl->second;
            poly.current_vertices[i] = c;
        }
        else
        {
            const vec3 c = (dr->second);
            poly.current_vertices[i] = c;
        }
    }
}

Polyhedron Data::GetSlicePolyhedronOfFrme(int frm_id, bool original) {
	if (faces.empty())
		return Polyhedron();

	int step = 0;
	vector<int3> triangles;
	vector<vec3> vertices;
	vector<vec3> correspondences;

	map<vec3, int, comp> mp;
	int vid = 1;

	// all video palette vertices
    const vector<vec4> &palette_vertices = original ? original_palette.vertices : current_palette.vertices;

	for (int face_id = 0; face_id < faces[frm_id].size(); face_id++) {

		//a face's TripleSegment(3 edges in the video palette)
		TripleSegment &ts = faces[frm_id][face_id];

		//3 edges:(ts.a,ts.b),(ts.c,ts.d),(ts.e,ts.f)
		vec3 ea(palette_vertices[ts.a][0], palette_vertices[ts.a][1], palette_vertices[ts.a][2]);
		vec3 eb(palette_vertices[ts.b][0], palette_vertices[ts.b][1], palette_vertices[ts.b][2]);
		vec3 ec(palette_vertices[ts.c][0], palette_vertices[ts.c][1], palette_vertices[ts.c][2]);
		vec3 ed(palette_vertices[ts.d][0], palette_vertices[ts.d][1], palette_vertices[ts.d][2]);
		vec3 ee(palette_vertices[ts.e][0], palette_vertices[ts.e][1], palette_vertices[ts.e][2]);
		vec3 ef(palette_vertices[ts.f][0], palette_vertices[ts.f][1], palette_vertices[ts.f][2]);

		//3 vertices p,q,r are calculated by interpolation
        vec3 p = ea * (1 - ts.w_ab) + eb * ts.w_ab;
		vec3 q = ec * (1 - ts.w_cd) + ed * ts.w_cd;
		vec3 r = ee * (1 - ts.w_ef) + ef * ts.w_ef;

		//number all face's vertices，record its corresponding edges in the video palette
		if (mp[p] == 0) {
			mp[p] = vid++;
			vertices.push_back(p);
			correspondences.push_back(vec3(ts.a, ts.b, (1 - ts.w_ab)));
		}
		if (mp[q] == 0) {
			mp[q] = vid++;
			vertices.push_back(q);
			correspondences.push_back(vec3(ts.c, ts.d, (1 - ts.w_cd)));
		}
		if (mp[r] == 0) {
			mp[r] = vid++;
			vertices.push_back(r);
			correspondences.push_back(vec3(ts.e, ts.f, (1 - ts.w_ef)));
		}

		//record these 3 vertices' indices
		triangles.push_back(int3(mp[p] - 1, mp[q] - 1, mp[r] - 1));
	}

	frm_vert_2_palette_vert_maps.push_back(mp);
	return Polyhedron(vertices, triangles, correspondences);
}

//Yili Wang's code
vector<float> Data::ComputeSingleVertexMVCWeightsForceInside(int time, int vid, const vec3 &x) {
	float min_distance = FLT_MAX;
	vec3 close_point;

	const Polyhedron& poly = slice_polys[time];
	for (int i = 0; i < poly.faces.size(); i++)
	{
		int3 triangle = poly.faces[i];
		vec3 triangle_vertices[3];

		triangle_vertices[0] = poly.vertices[triangle[0]];
		triangle_vertices[1] = poly.vertices[triangle[1]];
		triangle_vertices[2] = poly.vertices[triangle[2]];

		vec3 my_close_point = closesPointOnTriangle(triangle_vertices, x);
		float my_dis = (x - my_close_point).sqrnorm();

		if (my_dis < min_distance) {
			min_distance = my_dis;
			close_point = my_close_point;
		}
	}

	vec3 projection(close_point[0], close_point[1], close_point[2]);

    // qDebug() << x << "->" << projection;

	return ComputeSingleVertexMVCWeights(time, vid, projection);
}

void Data::resetVertex(int id) {
	current_palette.vertices[id] = original_palette.vertices[id];
}

void Data::resetAllVertex() {
    int n = current_palette.vertices.size();

    for (int i = 0; i < n; i++)
		current_palette.vertices[i] = original_palette.vertices[i];
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
        {
            colorstop[i][j].clear();
        }
}

void Data::ExportWeights(string path) {
	if (mixing_weights.empty())
		return;

	/*
	for (int i = 0; i < frame_cnt; i++) {
		ofstream ofs(path + "/weights_" + to_string(i) + ".txt");
		for (int j = 0; j < mixing_weights[i].size(); j++) {
			for (int k = 0; k < mixing_weights[i][j].size(); k++) {
				ofs << mixing_weights[i][j][k] << " ";
			}
			ofs << endl;
		}
		ofs.close();
	}
	*/
	
	for (int i = 0; i < frame_cnt; i++) {
		ofstream of(path + "/weights_" + to_string(i) + ".bin", ios::binary);
		for (int j = 0; j < mixing_weights[i].size(); j++) {
			of.write((char*)(&mixing_weights[i][j][0]), sizeof(float) * mixing_weights[i][j].size());
		}
		of.close();
	}
}

void Data::ImportWeights(string path) {
	mixing_weights.clear();
	mixing_weights.resize(frame_cnt);

	/*
	for (int i = 0; i < frame_cnt; i++) {
		mixing_weights[i].resize(frame_width*frame_height);
		ifstream ifs(path + "/weights_" + to_string(i) + ".txt");
		for (int j = 0; j < frame_width*frame_height; j++) {
			mixing_weights[i][j] = vector<float>(palette_vcnt, 0);
			for (int k = 0; k < palette_vcnt; k++) {
				ifs >> mixing_weights[i][j][k];
			}
		}
	}
	*/

	for (int i = 0; i < frame_cnt; i++) {
		mixing_weights[i].resize(frame_width*frame_height);
		ifstream ifs(path + "/weights_" + to_string(i) + ".bin", ios::binary);
		for (int j = 0; j < mixing_weights[i].size(); j++) {
			mixing_weights[i][j] = vector<float>(palette_vcnt, 0);
			ifs.read((char*)(&mixing_weights[i][j][0]), sizeof(float) * mixing_weights[i][j].size());
		}
		ifs.close();
	}

	is_weights_calculated = true;
}

void Data::ExportChangedPalette(string path) {

    timeSave = QTime::currentTime();

	ofstream ofs(path);
	ofs << current_palette.vertices.size() << endl;
	for (int i = 0; i < current_palette.vertices.size(); i++) {
		ofs << current_palette.vertices[i][0] << " " <<
			current_palette.vertices[i][1] << " " <<
			current_palette.vertices[i][2] << " " <<
            current_palette.vertices[i][3] << " " << vertex_palette_position[i] <<endl;
	}
    ofs << timeOpen.msecsSinceStartOfDay() << " "
        << timeFirstEdit.msecsSinceStartOfDay() << " "
        << timeLastEdit.msecsSinceStartOfDay() << " "
        << timeSave.msecsSinceStartOfDay() << endl;

    for (int i = 0; i < current_palette.vertices.size(); i++)
        for (int j = 0; j < current_palette.vertices.size(); j++)
        {
            const auto &colorstopij = colorstop[i][j];
            int n = 0;

            for(auto stop : colorstopij)
            {
                if(stop.first < 1e-8) continue;
                if(stop.first > 1-1e-8) continue;

                qDebug() << "color stop " << i << " " << j << " " << stop.first << " " << stop.second;
                n += 1;
            }

            ofs << n << "\n";

            for(auto stop : colorstopij)
            {
                if(stop.first < 1e-8) continue;
                if(stop.first > 1-1e-8) continue;

                n += 1;
                ofs << stop.first << " " << stop.second[0] << " " << stop.second[1] << " " << stop.second[2] << "\n";
            }
        }
}

void Data::ExportRecoloredVideo(string path) {
	if (!is_weights_calculated || mixing_weights.empty())
		return;

	QTime time;
	time.start();

	int k = 0;

	for (int t = 0; t < frame_cnt; t++) {
		QSize size(frame_width,frame_height);
        QImage image(size, QImage::Format_ARGB32);

        UpdateSlicePoly(t);

		#pragma omp parallel for num_threads(32)
		for (int i = 0; i < frame_width * frame_height * frame_depth; i += frame_depth)
		{
			vec3 x(
				original_video[t * frame_width * frame_height * frame_depth + i + 0],
				original_video[t * frame_width * frame_height * frame_depth + i + 1],
				original_video[t * frame_width * frame_height * frame_depth + i + 2]
			);

			vec3 y = RecolorSinglePixel(t, i / frame_depth, x);

            if(y[0] < 0) y[0] = 0;
            if(y[0] > 1) y[0] = 1;
            if(y[1] < 0) y[1] = 0;
            if(y[1] > 1) y[1] = 1;
            if(y[2] < 0) y[2] = 0;
            if(y[2] > 1) y[2] = 1;


			int row = i / (frame_width * frame_depth);
			int col = (i - row*frame_width * frame_depth) / frame_depth;
			image.setPixel(col, row, qRgb(y[0] * 255, y[1] * 255, y[2] * 255));

		}
		image.save((path + "/" + to_string(t) + ".png").c_str(), "PNG", 100);
	}
}

void Data::ExportOriginalVideo(string path) {
	if (!is_weights_calculated || mixing_weights.empty())
		return;

	QTime time;
	time.start();

	int k = 0;

	for (int t = 0; t < frame_cnt; t++) {
        QSize size(frame_width, frame_height);
		QImage image(size, QImage::Format_ARGB32);

		#pragma omp parallel for num_threads(32)
		for (int i = 0; i < frame_width * frame_height * frame_depth; i += frame_depth){
			vec3 x(
				original_video[t * frame_width * frame_height * frame_depth + i + 0],
				original_video[t * frame_width * frame_height * frame_depth + i + 1],
				original_video[t * frame_width * frame_height * frame_depth + i + 2]
			);

            int row = i / (frame_width * frame_depth);
            int col = (i - row*frame_width * frame_depth) / frame_depth;
			image.setPixel(col, row, qRgb(x[0] * 255, x[1] * 255, x[2] * 255));

		}
		image.save((path + "/" + to_string(t) + ".png").c_str(), "PNG", 100);
    }
}

const char *Tex[]={
    R"(
    )",
    R"(
    )",
};
void Data::ExportTikz(QString path, double preview)
{
    // QString mainTexFile = QDir::cleanPath(path + QDir::separator() + "input.tex");
    QDir(path).mkdir("original");
    QDir(path).mkdir("method");
    QDir(path + QDir::separator() + "method").mkdir("recolored");
    QDir(path + QDir::separator() + "method").mkdir("slice");

    ExportOriginalVideo((path + QDir::separator() + "original").toStdString());
    ExportRecoloredVideo((path + QDir::separator() + "method" + QDir::separator() + "recolored").toStdString());

    QMap<QPair<int, int>, int> start_id;
    start_id.clear();

    QString verticesFileName = QDir::cleanPath(path + QDir::separator() + "method" + QDir::separator() + "vertices.csv");
    QFile verticesFile( verticesFileName );

    if ( verticesFile.open(QIODevice::ReadWrite | QIODevice::Truncate) )
    {
        QTextStream stream( &verticesFile );
        stream.setRealNumberNotation(QTextStream::FixedNotation);

        for(int i = 0; i < current_palette.vertices.size(); i++)
        {
            vec4 d = current_palette.vertices[i] - original_palette.vertices[i];
            vec4 s = current_palette.vertices[i] + original_palette.vertices[i];

            int color = (s[0] + s[1] + s[2]) < 3.0f ? 1 : 2;
            int diff = (fabs(d[0]) < 1e-2 && fabs(d[1]) < 1e-2 && fabs(d[2]) < 1e-2) ? 0 : color;

            stream
                << i << ","
                << current_palette.vertices[i][3] * (this->frame_cnt - 1) << ","
                << 1 - vertex_palette_position[i] << ","
                << original_palette.vertices[i][0] << ","
                << original_palette.vertices[i][1] << ","
                << original_palette.vertices[i][2] << ","
                << current_palette.vertices[i][0] << ","
                << current_palette.vertices[i][1] << ","
                << current_palette.vertices[i][2] << ","
                << diff << ""
                << "\n";
        }


        int id = current_palette.vertices.size();

        for(int i = 0; i < current_palette.edges.size(); i++)
        {
            int u = current_palette.edges[i].u;
            int v = current_palette.edges[i].v;

            const vec4 &pu = original_palette.vertices[u];
            const vec4 &pv = original_palette.vertices[v];

            start_id[qMakePair(u, v)] = id;

            const auto &colorstopuv = colorstop[u][v];
            for(auto stop : colorstopuv)
            {
                if(stop.first < 1e-8) continue;
                if(stop.first > 1-1e-8) continue;

                float w = 1 - stop.first;

                const vec4 p = pu * w + pv * (1 - w);
                const vec3 s = vec3(p[0], p[1], p[2]) + stop.second;
                int color = (s[0] + s[1] + s[2]) < 3.0f ? 1 : 2;

                stream
                    << id << ","
                    << p[3] * (this->frame_cnt - 1) << ","
                    << 1 - w * vertex_palette_position[u] - (1 - w) * vertex_palette_position[v] << ","
                    << p[0] << ","
                    << p[1] << ","
                    << p[2] << ","
                    << stop.second[0] << ","
                    << stop.second[1] << ","
                    << stop.second[2] << ","
                    << -color << ""
                    << "\n";

                id += 1;
            }
        }

        verticesFile.close();
    }


    QString edgesFileName = QDir::cleanPath(path + QDir::separator() + "method" + QDir::separator() + "edges.csv");
    QFile edgesFile( edgesFileName );

    if ( edgesFile.open(QIODevice::ReadWrite | QIODevice::Truncate) )
    {
        QTextStream stream( &edgesFile );
        stream.setRealNumberNotation(QTextStream::FixedNotation);

        /*
        for(int i = 0; i < current_palette.edges.size(); i++)
        {
            stream
                << current_palette.edges[i].u << ","
                << current_palette.edges[i].v
                << "\n";
        }
        */

        int id = current_palette.vertices.size();
        for(int i = 0; i < current_palette.edges.size(); i++)
        {
            int u = current_palette.edges[i].u;
            int v = current_palette.edges[i].v;

            int last = u;

            const auto &colorstopuv = colorstop[u][v];
            for(auto stop : colorstopuv)
            {
                if(stop.first < 1e-8) continue;
                if(stop.first > 1-1e-8) continue;

                stream << last << "," << id << "\n";
                last = id;
                id += 1;
            }

            stream << last << "," << v << "\n";

        }

        edgesFile.close();
    }

    /*
    QString colorStopEdgeFileName = QDir::cleanPath(path + QDir::separator() + "method" + QDir::separator() + "colorstopedges.csv");
    QFile colorStopEdgeFile( colorStopEdgeFileName );

    if ( colorStopEdgeFile.open(QIODevice::ReadWrite | QIODevice::Truncate) )
    {
        QTextStream stream( &colorStopEdgeFile );
        stream.setRealNumberNotation(QTextStream::FixedNotation);
        int n = current_palette.vertices.size();

        for(int i = 0; i < current_palette.edges.size(); i++)
        {
            int u = current_palette.edges[i].u;
            int v = current_palette.edges[i].v;

            const std::pair<float, vec3> pu = make_pair(0.,
                vec3(
                    current_palette.vertices[u][0],
                    current_palette.vertices[u][1],
                    current_palette.vertices[u][2]
                )
            );

            const std::pair<float, vec3> pv = make_pair(1.,
                vec3(
                    current_palette.vertices[v][0],
                    current_palette.vertices[v][1],
                    current_palette.vertices[v][2]
                )
            );

            auto last = pu;

            const auto &colorstopuv = colorstop[u][v];

            bool isDouble = false;
            for(int k : {u, v})
            {
                vec4 d = current_palette.vertices[k] - original_palette.vertices[k];
                if(vec3(d[0], d[1], d[2]).norm() > 1e-2)
                    isDouble = true;
            }

            for(auto stop : colorstopuv)
            {
                if(stop.first < 1e-8) continue;
                if(stop.first > 1-1e-8) continue;

                isDouble = true;

                stream << u << "," <<
                          v << "," <<
                          last.first << "," <<
                          last.second[0] << "," <<
                          last.second[1] << "," <<
                          last.second[2] << "," <<
                          stop.first << "," <<
                          stop.second[0] << "," <<
                          stop.second[1] << "," <<
                          stop.second[2] << "," <<
                          (-1) << "\n";
                last = stop;
            }

            stream << u << "," <<
                      v << "," <<
                      last.first << "," <<
                      last.second[0] << "," <<
                      last.second[1] << "," <<
                      last.second[2] << "," <<
                      pv.first << "," <<
                      pv.second[0] << "," <<
                      pv.second[1] << "," <<
                      pv.second[2] << "," <<
                      (isDouble ? -1 : 0) << "\n";

            if(isDouble)
            {
                stream << u << "," <<
                          v << "," <<
                          0.0 << "," <<
                          original_palette.vertices[u][0] << "," <<
                          original_palette.vertices[u][1] << "," <<
                          original_palette.vertices[u][2] << "," <<
                          1.0 << "," <<
                          original_palette.vertices[v][0] << "," <<
                          original_palette.vertices[v][1] << "," <<
                          original_palette.vertices[v][2] << "," <<
                          (+1) << "\n";
            }
        }

        colorStopEdgeFile.close();
    }
    */

    for(int i = 0; i < frame_cnt; i++)
    {
        QString sliceFileName = QDir::cleanPath(path + QDir::separator() + "method" + QDir::separator() + "slice" + QDir::separator() + "sliceedge" + QString::number(i) + ".csv");
        QFile sliceFile( sliceFileName );
        if ( sliceFile.open(QIODevice::ReadWrite | QIODevice::Truncate) )
        {
            QTextStream stream( &sliceFile );
            stream.setRealNumberNotation(QTextStream::FixedNotation);

            const Polyhedron &slice_poly = slice_polys[i];
            for(int j = 0; j < slice_poly.faces.size(); j++)
            {
                int tmp[] = {slice_poly.faces[j].x, slice_poly.faces[j].y, slice_poly.faces[j].z};

                /*
                for(int uid : tmp)
                {
                    stream
                        << int(slice_poly.correspondences[uid].x() + 0.5) << ","
                        << (slice_poly.correspondences[uid].z()) << ","
                        << int(slice_poly.correspondences[uid].y() + 0.5) << ",";
                }
                */
                if(tmp[0] < tmp[1]) stream << tmp[0] << "," << tmp[1] << "\n";
                if(tmp[1] < tmp[2]) stream << tmp[1] << "," << tmp[2] << "\n";
                if(tmp[2] < tmp[0]) stream << tmp[2] << "," << tmp[0] << "\n";
                // stream << "0\n";

            }

            sliceFile.close();
        }
    }

    for(int i = 0; i < frame_cnt; i++)
    {
        QString sliceFileName = QDir::cleanPath(path + QDir::separator() + "method" + QDir::separator() + "slice" + QDir::separator() + "slicevertice" + QString::number(i) + ".csv");
        QFile sliceFile( sliceFileName );
        if ( sliceFile.open(QIODevice::ReadWrite | QIODevice::Truncate) )
        {
            QTextStream stream( &sliceFile );
            stream.setRealNumberNotation(QTextStream::FixedNotation);

            const Polyhedron &slice_poly = slice_polys[i];
            for(int j = 0; j < slice_poly.correspondences.size(); j++)
            {
                int u = int(slice_poly.correspondences[j].x() + 0.5);
                int v = int(slice_poly.correspondences[j].y() + 0.5);
                float w = 1 - (slice_poly.correspondences[j].z());

                const vec4 &u_ = current_palette.vertices[u];
                const vec4 &v_ = current_palette.vertices[v];
                auto &colorstopuv = colorstop[u][v];
                colorstopuv[0] = vec3(u_[0], u_[1], u_[2]);
                colorstopuv[1] = vec3(v_[0], v_[1], v_[2]);

                int last = u;
                float lastw = 0;
                int id = start_id[qMakePair(u, v)];
                bool ok = false;

                for(auto k : colorstopuv)
                {
                    if(k.first < 1e-8) continue;
                    if(k.first > 1-1e-8) continue;

                    if(lastw < w + 1e-6 && w < k.first + 1e-6)
                    {
                        stream
                            << last << ","
                            << 1 - (w - lastw) / (k.first - lastw) << ","
                            << id << "\n";
                        ok = true;
                        break;
                    }

                    lastw = k.first;
                    last = id;
                    id+=1;

                }

                if(!ok)
                {
                    // if(i == 48) qDebug() << "````" << w << lastw;
                    stream
                        << last << ","
                        << 1 - (w - lastw) / (1. - lastw) << ","
                        << v << "\n";
                }
            }

            sliceFile.close();
        }
    }

    /*
    for(int time = 0; time < frame_cnt; time++)
    {
        QString sliceFileName = QDir::cleanPath(path + QDir::separator() + "sliceoriginalvideo" + QString::number(time) + ".csv");
        QFile sliceFile( sliceFileName );
        if ( sliceFile.open(QIODevice::ReadWrite | QIODevice::Truncate) )
        {
            QTextStream stream( &sliceFile );
            stream.setRealNumberNotation(QTextStream::FixedNotation);
            stream.setRealNumberPrecision(3);

            QRandomGenerator rng;

            for (int k = 0; k < frame_width * frame_height; k++)
            {
                if (rng.bounded(1.) > preview) continue;

                double *p = original_video + time * frame_width * frame_height * frame_depth + k * frame_depth;
                double r = p[0], g = p[1], b = p[2];

                stream << r << "," << g << "," << b << "\n";
            }
        }

        sliceFile.close();
    }
    */


    for(int time = 0; time < frame_cnt; time++)
    {
        QString sliceFileName = QDir::cleanPath(path + QDir::separator() + "original" + QDir::separator() + QString::number(time) + ".texgen");
        QFile sliceFile( sliceFileName );
        if ( sliceFile.open(QIODevice::ReadWrite | QIODevice::Truncate) )
        {
            QTextStream stream( &sliceFile );
            stream.setRealNumberNotation(QTextStream::FixedNotation);
            stream.setRealNumberPrecision(3);

            QRandomGenerator rng;

            int *choose = new int[frame_width * frame_height];
            float *dis = new float[frame_width * frame_height];

            for (int k = 0; k < frame_width * frame_height; k++)
            {
                choose[k] = 0;
                dis[k] = 1e20f;
            }

            auto print = [&](int k){
                double *p = original_video + time * frame_width * frame_height * frame_depth + k * frame_depth;
                double r = p[0], g = p[1], b = p[2];


                // stream<<"\\fill[fill={rgb,1:red," << r << ";green," << g << ";blue," << b << "}](" << r << "," << g << "," << b << ")circle(\\pcsize);";
                stream
                    << "\\pgfpathcircle{\\pgfpointxyz{" << r << "}{" << g << "}{" << b << "}}{\\pcsize}"
                    << "\\color{rgb,1:red," << r << ";green," << g << ";blue," << b << "}\\pgfusepath{fill}\n";
            };

            for (int i = 0; i < 0 * 2; i++)
            {
                int ichoose = 0;
                for (int k = 1; k < frame_width * frame_height; k++)
                {
                    if(dis[ichoose] < dis[k]) ichoose = k;
                }

                print(ichoose);
                choose[ichoose]=1;
                dis[ichoose] = -1;

                float r0, g0, b0;
                {
                    double *p = original_video + time * frame_width * frame_height * frame_depth + ichoose * frame_depth;
                    r0 = p[0], g0 = p[1], b0 = p[2];
                }

                for (int k = 1; k < frame_width * frame_height; k++)
                {
                    double *p = original_video + time * frame_width * frame_height * frame_depth + k * frame_depth;
                    float r = p[0], g = p[1], b = p[2];

                    float newdisx = r - r0;
                    float newdisy = g - g0;
                    float newdisz = b - b0;
                    float newdis = newdisx * newdisx + newdisy * newdisy + newdisz * newdisz;

                    if(newdis < dis[k]) dis[k] = newdis;
                }
            }

            for (int k = 0; k < frame_width * frame_height; k++)
            {
                // if (choose[k]) continue;
                // if (rng.bounded(frame_width * frame_height) > 1024.0 * 2) continue;
                if (rng.bounded(1.) > preview) continue;

                //stream << "\\definecolor{c}{rgb}{" << r << "," << g << "," << b << "}\\fill[fill=c](" << r << "," << g << "," << b << ")circle(0.1pt);\n";

                print(k);

            }


            delete []choose;
            delete []dis;
        }

        sliceFile.close();
    }

    /*
    for(int time = 0; time < frame_cnt; time++)
    {
        QString sliceFileName = QDir::cleanPath(path + QDir::separator() + "bin" + QString::number(time) + ".tex");
        QFile sliceFile( sliceFileName );
        if ( sliceFile.open(QIODevice::ReadWrite | QIODevice::Truncate) )
        {
            int (*count)[16][16] = new int[16][16][16];

            memset(count, 0, sizeof(int) * 16 * 16 * 16);

            for (int k = 0; k < frame_width * frame_height; k++)
            {
                double *p = original_video + time * frame_width * frame_height * frame_depth + k * frame_depth;
                int r = int(p[0] * 255 + 0.5);
                int g = int(p[1] * 255 + 0.5);
                int b = int(p[2] * 255 + 0.5);
                count[(r >> 4) & 0xF][(g >> 4) & 0xF][(b >> 4) & 0xF] += 1;
            }

            QTextStream stream( &sliceFile );
            stream.setRealNumberNotation(QTextStream::FixedNotation);
            stream.setRealNumberPrecision(3);

            for(int i = 0; i < 16; i++)
                for(int j = 0; j < 16; j++)
                    for(int k = 0; k < 16; k++)
                    {
                        int c = count[i][j][k];
                        if(c == 0) continue;
                        // stream << i << j << k << c;
                        stream
                            << "\\definecolor{c}{rgb}{"
                            << i / 15.0 << "," << j/15.0 << "," << k/15.0
                            << "}\\fill[fill=c]("
                            << i / 15.0 << "," << j/15.0 << "," << k/15.0
                            << ")circle(" << c / double(frame_width * frame_height) * 10 << "pt);\n";
                    }

            delete []count;
        }
    }

    for(int time = 0; time < frame_cnt; time++)
    {
        QString sliceFileName = QDir::cleanPath(path + QDir::separator() + "bin" + QString::number(time) + ".csv");
        QFile sliceFile( sliceFileName );
        if ( sliceFile.open(QIODevice::ReadWrite | QIODevice::Truncate) )
        {
            int (*count)[16][16] = new int[16][16][16];

            memset(count, 0, sizeof(int) * 16 * 16 * 16);

            for (int k = 0; k < frame_width * frame_height; k++)
            {
                double *p = original_video + time * frame_width * frame_height * frame_depth + k * frame_depth;
                int r = int(p[0] * 255 + 0.5);
                int g = int(p[1] * 255 + 0.5);
                int b = int(p[2] * 255 + 0.5);
                count[(r >> 4) & 0xF][(g >> 4) & 0xF][(b >> 4) & 0xF] += 1;
            }

            QTextStream stream( &sliceFile );
            stream.setRealNumberNotation(QTextStream::FixedNotation);
            stream.setRealNumberPrecision(3);

            for(int i = 0; i < 16; i++)
                for(int j = 0; j < 16; j++)
                    for(int k = 0; k < 16; k++)
                    {
                        int c = count[i][j][k];
                        if(c == 0) continue;
                        stream << i / 15.0 << "," << j / 15.0 << "," << k / 15.0 << "," << c << endl;
                    }
        }
    }
    */

    QMessageBox::information(NULL, "Info", "Export TikZ Data finished", QMessageBox::Ok, QMessageBox::Ok);
}

vector<vec3> Data::ReadFramePalette(int fid) {

	vector<vec3> frm_verts;
	string frm_palette_path = base_dir + "/frm_palette_%05d.obj" ;
	char s[128];
	sprintf(s, frm_palette_path.c_str(), fid);

	ifstream in(s);
	double v1, v2, v3;
	char c;
	while (!in.eof()) {
		in >> c;
		if (c == 'v') {
			in >> v1 >> v2 >> v3;
			vec3 vert(v1, v2, v3);
			frm_verts.push_back(vert);
		}
		else
			break;
	}
	return frm_verts;
}

const vector<vector<map<double, vec3> > > &Data::getColorStop() const
{
    return colorstop;
}

void Data::setColorStop(int x, int y, double w, vec3 c)
{
    if(1e-2 < w && w < 1 - 1e-2)
    {
        if(!hasColorStop)
        {
            is_weights_calculated = false;
            hasColorStop = true;
        }
    }
    colorstop[x][y][w] = c;
}

const Polyhedron& Data::getSlice_polys(int i) const
{
    if(0 <= i && i < slice_polys.size())
    {
        return slice_polys[i];
    }
    else
    {
        static Polyhedron empty;
        return empty;
    }
}

void Data::ImportChangedPalette(string path) {
    ifstream ifs(path);

    const bool isV2 = QString(path.c_str()).endsWith("palettev2");
    const bool isV3 = QString(path.c_str()).endsWith("palettev3");
    const bool isV4 = QString(path.c_str()).endsWith("palettev4");

    if(isV2 || isV3 || isV4)
    {
        int n; ifs >> n;
        for (int i = 0; i < n; i++)
        {
            ifs >> current_palette.vertices[i].v[0] >> current_palette.vertices[i].v[1]
                >> current_palette.vertices[i].v[2] >> current_palette.vertices[i].v[3] >> vertex_palette_position[i];
        }

        if(isV3 || isV4)
        {
            int p, q, r, s;
            ifs >> p >> q >> r >> s;
        }

        if(isV4)
        {
            qDebug() << "isV4!!!!!!!!";
            for(int i = 0; i < n; i++)
                for(int j = 0; j < n; j++)
                {
                    int m;
                    ifs >> m;
                    for(int k = 0; k < m; k++)
                    {
                        double w, r, g, b;

                        ifs >> w >> r >> g >> b;

                        colorstop[i][j][w] = vec3(r, g, b);
                    }
                }
        }
    }
    else
    {
        int n; ifs >> n;
        for (int i = 0; i < n; i++)
        {
            ifs >> current_palette.vertices[i].v[0] >> current_palette.vertices[i].v[1]
                >> current_palette.vertices[i].v[2] >> current_palette.vertices[i].v[3];
        }
    }
	emit updated();
    isEdited = false;
}


void Data::recalcPaletteHeight(){
	const vector<vec4> &cur_video_palette = current_palette.vertices;
	const vector<int2> &palette_edges = current_palette.edges;
	int n_step = 1;

	for (int t = 0; t < n_step; t++){
		int n = vertex_palette_position.size();
		vector<int> order(n);
		vector<double> hue(n);
		vector<double> time(n);

		for (int i = 0; i < n; i++){
			order[i] = i;

			vec4 p = cur_video_palette[i];
			double hp, sp, vp;
			RGBtoHSV(p[0], p[1], p[2], hp, sp, vp);

			if (hp < 30) hp += 1000;

			const double eps = .15;

			if (p[0] > 1 - eps && p[1] > 1 - eps && p[2] > 1 - eps) hp = 1e8;
			if (p[0] < eps && p[1] < eps && p[2] < eps) hp = -1e8;

			hue[i] = hp;
			time[i] = p[3];
		}

		std::sort(order.begin(), order.end(), [=](int i, int j) -> bool {
			if (abs(time[i] - time[j]) > 1e-6) return time[i] < time[j];
			return hue[i] < hue[j];
		}
		);

		vector<int> cnt(frame_cnt);
		vector<int> id(n);
		int max_n = 0;

		for (int p = 0; p < n; p++){
			int i = order[p];
			const vec4 &r = cur_video_palette[i];

            int t = static_cast<int>(r[3] * (frame_cnt - 1) + .5f);
			id[i] = cnt[t];
			cnt[t] += 1;

			if (max_n < cnt[t]) max_n = cnt[t];
		}

		for (int p = 0; p < n; p++){
			int i = order[p];
			const vec4 &r = cur_video_palette[i];
            int t = static_cast<int>(r[3] * (frame_cnt - 1) + .5f);

            if (t == 0 || t == frame_cnt - 1) continue;

			int best = -1;
			double best_dif = 1e60;
			for (int k = 0; k < max_n; k++){
				bool ok = true;
				double dif = 1e60;
				for (int q = 0; q < n; q++){
					int j = order[q];
					const vec4 &s = cur_video_palette[j];
                    int tj = static_cast<int>(s[3] * (frame_cnt - 1) + .5f);

					if (t == tj){
						if (id[j] == k){
							ok = false;
							break;
						}
					}
					else{
						if (id[j] == k){
							double idif = abs(hue[i] - hue[j]);
							if (dif > idif) dif = idif;
						}
					}
				}

				if (best_dif > dif){
					best_dif = dif;
					best = k;
				}

				qDebug() << "dif << best_dif" << dif << best_dif;
			}
			id[i] = best;
		}

		for (int i = 0; i < n; i++){
			vertex_palette_position[i] = (id[i] + 0.5) / (double)max_n;
		}
	}
}
