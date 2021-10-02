#include "rgbwidget.h"
#include<QDebug>
#include<QFile>
#include<QRandomGenerator>
#include<QTimer>
#include"utility.h"

// Widget shows a RGB view

RGBWidget::RGBWidget(bool _original) : original(_original), sharedMemory("RGBWidget")
{
    QTimer *timer = new QTimer(this);

    if(!sharedMemory.attach())
    {
        sharedMemory.create(sizeof(float[3]));
    }

    connect(timer, &QTimer::timeout, [=](){
#ifndef USER_STUDY
        sharedMemory.lock();
        const float *data = (float*)sharedMemory.constData();
        rotate_x = data[0];
        rotate_y = data[1];
        rotate_z = data[2];
        sharedMemory.unlock();
#endif
        update();
    });

    timer->start(40);
}

void RGBWidget::paintGL()
{
	OpenGL3DWidget::paintGL();


    glLineWidth(.5f);
	glBegin(GL_LINES);

	for (int p = 0; p <= 1; p++)
		for (int r = 0; r <= 1; r++)
		{
			double q = p - .5;
			double s = r - .5;

			glColor3f(p, r, 0.);
			glVertex3f(q, s, -.5);
			glColor3f(p, r, 1.);
			glVertex3f(q, s, .5);
			glColor3f(p, 0., r);
			glVertex3f(q, -.5, s);
			glColor3f(p, 1., r);
			glVertex3f(q, .5, s);
			glColor3f(0., p, r);
			glVertex3f(-.5, q, s);
			glColor3f(1., p, r);
			glVertex3f(.5, q, s);
		}

	glEnd();

	if (data == nullptr) return;


	int frame_cnt = data->getFrmCnt();
	int width = data->getFrmWidth();
	int height = data->getFrmHeight();
	int depth = data->getFrmDepth();

	double *video = data->getVideo(true);
	if(original)
		video = data->getVideo(false);


    // vector<vec4> palette_vertices = data->getCurVideoPalette().vertices;
    // vector<int2> palette_edges = data->getCurVideoPalette().edges;

	if (showVideoData)
	{
		QRandomGenerator rng;

		glPointSize(2.f);
		glBegin(GL_POINTS);

		for (int i = 0; i < width * height; i++)
		{
			if (rng.bounded(1.) > preview) continue;

			double *p = video + time * width * height * depth + i * depth;
			double r = p[0], g = p[1], b = p[2];

			glColor3f(r, g, b);
			glVertex3f(r - .5, g - .5, b - .5);
		}

		glEnd();
	}

    const Polyhedron &poly = data->getSlice_polys(time);  //data->slice_polys; // data->GetSlicePolyhedronOfFrme(time, original);
    int fcnt = poly.faces.size();
    int vcnt = poly.vertices.size();//   faces.size();

    glPointSize(10.f);
	glBegin(GL_POINTS);

    const vector<vec3> &vertices = original ? poly.vertices : poly.current_vertices;

    for (int i = 0; i < vcnt; i++)
    {
        const vec3 &p = vertices[i];

        glColor3f(p[0], p[1], p[2]);
        glVertex3f(p[0] - .5, p[1] - .5, p[2] - .5);
	}
	glEnd();

	glLineWidth(3.f);
	glBegin(GL_LINES);


	for (int i = 0; i < fcnt; i++)
	{
		int vid1 = poly.faces[i].x;
		int vid2 = poly.faces[i].y;
		int vid3 = poly.faces[i].z;

        vec3 p = vertices[vid1];
        vec3 q = vertices[vid2];
        vec3 r = vertices[vid3];

		double center[3] = {
			(p[0] + q[0] + r[0]) / 3.,
			(p[1] + q[1] + r[1]) / 3.,
			(p[2] + q[2] + r[2]) / 3.,
		};

		glColor3f(p[0], p[1], p[2]);
		glVertex3f(p[0] - .5, p[1] - .5, p[2] - .5);

		glColor3f(q[0], q[1], q[2]);
		glVertex3f(q[0] - .5, q[1] - .5, q[2] - .5);

		glColor3f(p[0], p[1], p[2]);
		glVertex3f(p[0] - .5, p[1] - .5, p[2] - .5);

		glColor3f(r[0], r[1], r[2]);
		glVertex3f(r[0] - .5, r[1] - .5, r[2] - .5);

		glColor3f(q[0], q[1], q[2]);
		glVertex3f(q[0] - .5, q[1] - .5, q[2] - .5);

		glColor3f(r[0], r[1], r[2]);
		glVertex3f(r[0] - .5, r[1] - .5, r[2] - .5);

		glColor3f(center[0], center[1], center[2]);


		double d1[3] = {
			q[0] - p[0],
			q[1] - p[1],
			q[2] - p[2],
		};
		double d2[3] = {
			r[0] - p[0],
			r[1] - p[1],
			r[2] - p[2],
		};
		double n[3] = {
			d1[1] * d2[2] - d1[2] * d2[1],
			d1[2] * d2[0] - d1[0] * d2[2],
			d1[0] * d2[1] - d1[1] * d2[0],
		};

        double norm = n[0] * n[0] + n[1] * n[1] + n[2] * n[2];

#ifndef USER_STUDY
        // qDebug() << "DEBUG norm" << norm;

		if (norm > 1e-6)
		{
			/*
            double k = 0.1 / mysqrt(norm);
			n[0] *= k;
			n[1] *= k;
			n[2] *= k;

			glColor3f(center[0], center[1], center[2]);
			glVertex3f(center[0] - .5, center[1] - .5, center[2] - .5);

			glColor3f(center[0], center[1], center[2]);
			glVertex3f(center[0] + n[0] - .5, center[1] + n[1] - .5, center[2] + n[2] - .5);
			*/
		}
#endif
	}
	glEnd();
	glFlush();
    this->makeCurrent();
}

void RGBWidget::resizeGL(int w, int h)
{
    // qDebug() << ("resizeGL");
    // int m = w < h ? w : h;
    // glViewport(m * 0.4, m * 0.4, m * 0.6, m * 0.6);
    // update();
}

void RGBWidget::mousePressEvent(QMouseEvent *event)
{
    OpenGL3DWidget::mousePressEvent(event);
#ifndef USER_STUDY
    sharedMemory.lock();
    float *data = (float*)sharedMemory.data();
    data[0] = rotate_x;
    data[1] = rotate_y;
    data[2] = rotate_z;
    sharedMemory.unlock();
#endif
}

void RGBWidget::mouseMoveEvent(QMouseEvent *event)
{
    OpenGL3DWidget::mouseMoveEvent(event);
#ifndef USER_STUDY
    sharedMemory.lock();
    float *data = (float*)sharedMemory.data();
    data[0] = rotate_x;
    data[1] = rotate_y;
    data[2] = rotate_z;
    sharedMemory.unlock();
#endif
}

double RGBWidget::_rotate_x = -15.;
double RGBWidget::_rotate_y = -150.;
double RGBWidget::_rotate_z = 0.;
