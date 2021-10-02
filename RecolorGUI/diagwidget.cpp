#include "diagwidget.h"
#include<cmath>
#include<QWheelEvent>
#include <QRandomGenerator>
#include"utility.h"

// Widget shows a diag view

DiagWidget::DiagWidget(QWidget *parent) : OpenGL3DWidget(parent)
{
}

void DiagWidget::wheelEvent(QWheelEvent *event)
{
    /*
    OpenGL3DWidget::wheelEvent(event);
    // qDebug() << "wheelEvent" << event->x() << event->y() << event->delta();
    logWindow += event->delta() * 0.05;

    update();
    */
}

void DiagWidget::setWindow(double logWindow)
{
    this->logWindow = - logWindow / 1000.;
    update();
}


static void hsv2rgb(double r, double g, double b, double &_l, double &_a, double &_b)
{
    const static double sqrt3 = mysqrt(3);
    const static double sqrt2 = mysqrt(2);
    const static double sqrt6 = mysqrt(6);

    _l = r / sqrt3 + g / sqrt3 + b / sqrt3;
    _a = g / sqrt2 - r / sqrt2;
    _b = - g / sqrt6 - r / sqrt6 + 2 * b / sqrt6;

    _l /= sqrt3;
    _a /= sqrt3;
    _b /= sqrt3;
}

void DiagWidget::paintGL()
{
    OpenGL3DWidget::paintGL();

    glBegin(GL_LINES);

    for(int p = 0; p <= 1; p++)
        for(int r = 0; r <= 1; r++)
        {
            double q = p - .5;
            double s = r - .5;

            glColor3f(1., 1., 1.);
            glVertex3f(q, s, -.5);
            glColor3f(1., 1., 1.);
            glVertex3f(q, s, .5);
            glColor3f(1., 1., 1.);
            glVertex3f(q, -.5, s);
            glColor3f(1., 1., 1.);
            glVertex3f(q, .5, s);
            glColor3f(1., 1., 1.);
            glVertex3f(-.5, q, s);
            glColor3f(1., 1., 1.);
            glVertex3f(.5, q, s);
        }

    glEnd();

    if(data == nullptr) return;

    int frame_cnt = data->getFrmCnt();
    int width = data->getFrmWidth();
    int height = data->getFrmHeight();
    int depth = data->getFrmDepth();
    double *video = data->getVideo();


	vector<vec4> palette_vertices = data->getCurVideoPalette().vertices;
	vector<int2> palette_edges = data->getCurVideoPalette().edges;

    double window = exp(logWindow);

    glShadeModel(GL_SMOOTH);
    if(showVideoData)
    {
        QRandomGenerator rng;
        glPointSize(1.f);

        glBegin(GL_POINTS);

        for(int i = 0; i < width * height * frame_cnt; i++)
        {
            if(rng.bounded(1.) > preview) continue;

            double *p = video + i * depth;
            double r = p[0], g = p[1], b = p[2], t = (i / (width * height));

            double _l, _a, _b;

            hsv2rgb(r, g, b, _l, _a, _b);

            // qDebug() << _l << _a << _b;

            double alpha = exp(-(time - t) * (time - t) * window) * .995 + .005;

            glColor4f(r, g, b, alpha);
            glVertex3f(t / double(frame_cnt - 1)  - .5, _a, _b);
        }

        glEnd();
    }

    glPointSize(10.f);
    glBegin(GL_POINTS);

    for(int i = 0; i < palette_edges.size(); i++)
    {
		int uid = palette_edges[i][0];
		int vid = palette_edges[i][1];
		vec4 p = palette_vertices[uid];
		vec4 q = palette_vertices[vid];

        double r = 0, g = 0, b = 0;

        if(cut(time, p, q, r, g, b))
        {
            double _l, _a, _b;

            hsv2rgb(r, g, b, _l, _a, _b);

            glColor3f(r, g, b);
            glVertex3f(time / double(frame_cnt - 1)  - .5, _a, _b);
        }
    }

    glEnd();


    glPointSize(5.f);
    glBegin(GL_POINTS);

    for(int i = 0; i < palette_vertices.size(); i++)
    {
		vec4 p = palette_vertices[i];
        double r = p[0], g = p[1], b = p[2], t = (p[3]);

        double _l, _a, _b;

        double alpha = exp(-(time - t) * (time - t) * window) * .995 + .005;

        hsv2rgb(r, g, b, _l, _a, _b);

        glColor4f(r, g, b, alpha);
        glVertex3f(t / double(frame_cnt - 1) - .5, _a, _b);
    }
    glEnd();


    glShadeModel(GL_SMOOTH);
    glBegin(GL_LINES);
    for(int i = 0; i < palette_edges.size(); i++)
    {
		int uid = palette_edges[i][0];
		int vid = palette_edges[i][1];
		vec4 p = palette_vertices[uid];
		vec4 q = palette_vertices[vid];

        double rp = p[0], gp = p[1], bp = p[2];
        double rq = q[0], gq = q[1], bq = q[2];

        double _lp, _ap, _bp;
        double _lq, _aq, _bq;

        double tp = p[3];
        double tq = q[3];

        double alphap = exp(-(time - tp) * (time - tp) * window) * .995 + .005;
        double alphaq = exp(-(time - tq) * (time - tq) * window) * .995 + .005;

        hsv2rgb(rp, gp, bp, _lp, _ap, _bp);
        hsv2rgb(rq, gq, bq, _lq, _aq, _bq);

        glColor4f(rp, gp, bp, alphap);
        glVertex3f(tp / double(frame_cnt - 1) - .5, _ap, _bp);
        glColor4f(rq, gq, bq, alphaq);
        glVertex3f(tq / double(frame_cnt - 1) - .5, _aq, _bq);
    }

    glEnd();


    glBegin(GL_POLYGON);

    glColor4f(1.0f, 1.0f, 1.0f, 0.2f);
    glVertex3f(time / double(frame_cnt - 1) - .5, -0.5, -0.5);

    glColor4f(1.0f, 1.0f, 1.0f, 0.2f);
    glVertex3f(time / double(frame_cnt - 1) - .5, -0.5, 0.5);

    glColor4f(1.0f, 1.0f, 1.0f, 0.2f);
    glVertex3f(time / double(frame_cnt - 1) - .5, 0.5, 0.5);

    glColor4f(1.0f, 1.0f, 1.0f, 0.2f);
    glVertex3f(time / double(frame_cnt - 1) - .5, 0.5, -0.5);

    glEnd( );

    glFlush();
    this->makeCurrent();
}
