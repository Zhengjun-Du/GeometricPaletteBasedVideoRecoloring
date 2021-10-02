#include "paletteviewwidget.h"

#include<QTimer>
#include<QDebug>
#include<cmath>
#include<QMouseEvent>
#include<QCryptographicHash>
#include <QColorDialog>
#include <QMenu>
#include "qrgb.h"
#include "math.h"
#include <algorithm>
#include <QGuiApplication>
#ifdef _WIN32
#  include "Qt-Color-Widgets\include\QtColorWidgets\color_dialog.hpp"
#else
#  include "QtColorWidgets/color_dialog.hpp"
#endif
using namespace std;
using namespace color_widgets;

// Widget shows a palette-time view

PaletteViewWidget::PaletteViewWidget(QWidget *parent) : OpenGLWidget(parent)
{
    // QTimer *timer = new QTimer(this);
    // connect(timer, &QTimer::timeout, [=](){ blink = !blink; update(); });
    // timer->start(800);

    pMenu = new QMenu(this);
    QAction *pResetTask = new QAction(tr("reset this color"), this);
    QAction *pResetAllTask = new QAction(tr("reset all color"), this);
    pMenu->addAction(pResetTask);
    pMenu->addAction(pResetAllTask);

    connect(pResetTask, SIGNAL(triggered()), this, SLOT(resetVertex()));
    connect(pResetAllTask, SIGNAL(triggered()), this, SLOT(resetAllVertex()));
}

void PaletteViewWidget::setTime(int t) {
    time = t;
    data->Recolor(time);
    update();
}

void PaletteViewWidget::getColor(QColor c) {
    int r = qRed(c.rgb());
    int g = qGreen(c.rgb());
    int b = qBlue(c.rgb());

    const vector<int2> &palette_edges = data->getOriVideoPalette().edges;
    const vector<vec4> &palette_vertices = data->getCurVideoPalette().vertices;

    if (selected_vid != -1) {
        data->setVertex(selected_vid, c);
        update();
        data->Recolor(time);
    }
    else if(selected_eid != -1)
    {
        int uid = palette_edges[selected_eid][0];
        int vid = palette_edges[selected_eid][1];

        if (palette_vertices[uid][3] > palette_vertices[vid][3]) std::swap(uid, vid);
        double w = (time / (double)(data->getFrmCnt()-1) - palette_vertices[uid][3]) / (palette_vertices[vid][3] - palette_vertices[uid][3]);

        if(1e-2 < w && w < 1 - 1e-2)
            data->setColorStop(uid, vid, w, vec3(r / 255., g / 255., b / 255.));

        update();
        data->Recolor(time);
    }
}

void PaletteViewWidget::paintGL()
{
    OpenGLWidget::paintGL();

    glScalef(scale, scale, scale);

    int w = width(), h = height();
    double aspect = w*1.0 / h;

    //绘制白色矩形框
    glLineWidth(0.5f);
    glBegin(GL_LINES);

    for (int p = 0; p <= 1; p++)
    {
        double q = p - .5;
        glColor3f(1., 1., 1.);
        glVertex3f(q, -.5, 0.5);
        glColor3f(1., 1., 1.);
        glVertex3f(q, .5, 0.5);
        glColor3f(1., 1., 1.);
        glVertex3f(-.5, q, 0.5);
        glColor3f(1., 1., 1.);
        glVertex3f(.5, q, 0.5);
    }

    if (data == nullptr) return;

    int frame_cnt = data->getFrmCnt();
    int width = data->getFrmWidth();
    int height = data->getFrmHeight();
    int depth = data->getFrmDepth();
    double *video = data->getVideo();

    glBegin(GL_LINES);
    glColor4f(1., 1., 1., 1.);
    glVertex3f(time / (double)(frame_cnt - 1) - .5, -.5, .5);
    glVertex3f(time / (double)(frame_cnt - 1) - .5, .5, .5);
    glEnd();

    glColor4f(1., 1., 1., 1.);
    glBegin(GL_LINES);
    glVertex3f(time / (double)(frame_cnt - 1) - .5, -.5, .5);
    glVertex3f(time / (double)(frame_cnt - 1) - .5 - .01, -.5 - 0.01*aspect, .5);
    glVertex3f(time / (double)(frame_cnt - 1) - .5 - .01, -.5 - 0.01*aspect, .5);
    glVertex3f(time / (double)(frame_cnt - 1) - .5 + .01, -.5 - 0.01*aspect, .5);
    glVertex3f(time / (double)(frame_cnt - 1) - .5 + .01, -.5 - 0.01*aspect, .5);
    glVertex3f(time / (double)(frame_cnt - 1) - .5, -.5, .5);


    glVertex3f(time / (double)(frame_cnt - 1) - .5, +.5, .5);
    glVertex3f(time / (double)(frame_cnt - 1) - .5 - .01, +.5 + 0.01*aspect, .5);
    glVertex3f(time / (double)(frame_cnt - 1) - .5 - .01, +.5 + 0.01*aspect, .5);
    glVertex3f(time / (double)(frame_cnt - 1) - .5 + .01, +.5 + 0.01*aspect, .5);
    glVertex3f(time / (double)(frame_cnt - 1) - .5 + .01, +.5 + 0.01*aspect, .5);
    glVertex3f(time / (double)(frame_cnt - 1) - .5, +.5, .5);
    glEnd();

    const double *vertex_palette_position = data->getCurVideoPalette_palette_position();
    const vector<vec4> &cur_vertex = data->getCurVideoPalette().vertices;
    const vector<vec4> &ori_vertex = data->getOriVideoPalette().vertices;
    const vector<int2> &palette_edges = data->getOriVideoPalette().edges;

    const vector<vec4> &up_vertex = isShowBefore ? ori_vertex : cur_vertex;
    const vector<vec4> &down_vertex = isShowAfter ? cur_vertex : ori_vertex;

    const vector<vector<map<double, vec3> > > &colorstop = data->getColorStop();


#ifdef _WIN32
    double ha = 0.012 * 1.2;
    float ha_selected = 0.014 * 1.2;
#else
    double ha = 0.02 * 0.5 * 1.2 * 800 / w;
    float ha_selected = 0.025 * 0.5 * 1.2 * 800 / w;
#endif

    // qDebug() << "^^ width" << w;

    ha *= 0.95;

    for (int i = 0; i < palette_edges.size(); i++)
    {
        int uid = palette_edges[i][0];
        int vid = palette_edges[i][1];

        if (ori_vertex[uid][3] > ori_vertex[vid][3]) std::swap(uid, vid);

        const vec4 &up = up_vertex[uid];
        const vec4 &uq = up_vertex[vid];
        const vec4 &dp = down_vertex[uid];
        const vec4 &dq = down_vertex[vid];

        double hp = vertex_palette_position[uid];
        double hq = vertex_palette_position[vid];

        // double rp = p[0], gp = p[1], bp = p[2];
        // double rq = q[0], gq = q[1], bq = q[2];

        // double tp = p[3];
        // double tq = q[3];

        double alphap = 1.;// exp(-(time - tp) * (time - tp) * window) * .99 + .01;
        double alphaq = 1.;// exp(-(time - tq) * (time - tq) * window) * .99 + .01;

        const map<double, vec3> &colorstopuv = colorstop[uid][vid];

        data->setColorStop(uid, vid, 0, vec3(dp[0], dp[1], dp[2]));
        data->setColorStop(uid, vid, 1, vec3(dq[0], dq[1], dq[2]));

#ifndef USER_STUDY
        if(selected_eid == i)
            glLineWidth(1.f);
        else
            glLineWidth(.5f);
#else
        glLineWidth(.5f);
#endif


        glShadeModel(GL_SMOOTH);

        if(isShowAfter)
        {
            auto p = colorstopuv.begin();
            auto q = p; q++;
            for(; q != colorstopuv.end(); p++, q++)
            {
                const vec3& c0 = p->second;
                const vec3& c1 = q->second;
                const double w0 = p->first;
                const double w1 = q->first;
                const double t0 = up[3] * (1 - w0) + uq[3] * w0;
                const double t1 = up[3] * (1 - w1) + uq[3] * w1;
                const double h0 = hp * (1 - w0) + hq * w0;
                const double h1 = hp * (1 - w1) + hq * w1;

                glBegin(GL_LINES);

                glColor4f(c0[0], c0[1], c0[2], 1.f);
                glVertex3f((t0) - .5, (h0) - .5f + (isShowBefore ? -6e-3f : 0), .5);

                glColor4f(c1[0], c1[1], c1[2], 1.f);
                glVertex3f((t1) - .5, (h1) - .5f + (isShowBefore ? -6e-3f : 0), .5);
                glEnd();
            }
        }

        if (isShowBefore)
        {
            glBegin(GL_LINES);
            glColor4f(up[0], up[1], up[2], 1.f);
            glVertex3f(up[3] - .5, (hp) - .5f + (isShowAfter ? +6e-3f : 0), .5);
            glColor4f(uq[0], uq[1], uq[2], 1.f);
            glVertex3f(uq[3] - .5, (hq) - .5f + (isShowAfter ? +6e-3f : 0), .5);
            glEnd();
        }

        ha *= 0.85;

        for(auto stop : colorstopuv)
        {
            const double w = stop.first;

            if(w < 1e-2 || w > 1-1e-2) continue;

            const vec3 &cn = stop.second;
            vec4 co_ = up * (1 - w) + uq * (w);
            vec3 co(co_[0], co_[1], co_[2]);
            double t = up[3] * (1 - w) + uq[3] * w;
            double h = hp * (1 - w) + hq * w;
            double scale = 1;

            vec3 cd = (!isShowAfter) ? co : cn;
            vec3 cu = (!isShowBefore) ? cn : co;

            glColor4f(cu[0], cu[1], cu[2], alphap);
            glBegin(GL_POLYGON);
            glVertex3f(t - .5, h - .5, 0.);
            for(int k = 0; k <= 2; k++)
            {
                double c = cos(k / 2. * M_PI); // c = c * c * c;
                double s = sin(k / 2. * M_PI); // s = s * s * s;
                glVertex3f(t - .5 + c * ha *  scale, h - .5 + s * ha * scale * aspect , 0.);
            }
            glVertex3f(t - .5 + 1 * ha *  scale, h - .5 + 0 * ha * scale * aspect , 0.);
            glEnd();

            glColor4f(cd[0], cd[1], cd[2], alphap);
            glBegin(GL_POLYGON);
            glVertex3f(t - .5, h - .5, 0.);
            for(int k = 2; k <= 4; k++)
            {
                double c = cos(k / 2. * M_PI); // c = c * c * c;
                double s = sin(k / 2. * M_PI); // s = s * s * s;
                glVertex3f(t - .5 + c * ha *  scale, h - .5 + s * ha * scale * aspect , 0.);
            }
            glEnd();

            // draw arrow
            if(abs(cu[0] - cd[0]) + abs(cu[1] - cd[1]) + abs(cu[2] - cd[2]) > 5e-2)
            {
                if(cu[0] + cu[1] + cu[2] + cd[0] + cd[1] + cd[2] < 3.f)
                    glColor4f(1.f, 1.f, 1.f, 1.f);
                else
                    glColor4f(0.f, 0.f, 0.f, 1.f);

                double x = t - 0.5;
                double y = h - 0.5;

                glLineWidth(1.f);
                glBegin(GL_LINES);
                glVertex3f(x, y, 0.);
                glVertex3f(x, y + ha * 0.55 * aspect, 0.);
                glEnd();

                glLineWidth(1.f);
                glBegin(GL_LINES);
                glVertex3f(x, y, 0.);
                glVertex3f(x, y - ha * 0.65 * aspect, 0.);

                glVertex3f(x - 0.45 * ha, y - ha * 0.05 * aspect, 0.);
                glVertex3f(x, y - ha * 0.65 * aspect, 0.);

                glVertex3f(x + 0.45 * ha, y - ha * 0.05 * aspect, 0.);
                glVertex3f(x, y - ha * 0.65 * aspect, 0.);
                glEnd();
            }
        }

        ha /= 0.85;
        ha *= 0.8;

        if (up[3] + 1e-2 < time / (double)(frame_cnt - 1) && time / (double)(frame_cnt - 1) + 1e-2 < uq[3])
        {
            /*
            float w1 = (tq - tp)
            float t1 = (w1) * (tq - tp) + tp;
            float h1 = hp * (1 - w1) + hq * w1;
            float w2 = j.key();
            vec3 rgb1 =
            */
            // float or1 = rp * (1 - w1) + rq * w1;
            // float og1 = gp * (1 - w1) + gq * w1;
            // float ob1 = bp * (1 - w1) + bq * w1;
            // const vec3 &originalColorU = data->getOriVideoPalette().vertices[uid];
            // const vec3 &originalColorV = data->getOriVideoPalette().vertices[vid];
            // vec3 originalColor = originalColorU * (1 - w1) + originalColorV * w1;
            /*
            */

            float w = (time / (double)(frame_cnt - 1) - up[3]) / (uq[3] - up[3]);

            vec4 co_ = up * (1 - w) + uq * w;
            vec3 co(co_[0], co_[1], co_[2]);

            // vec4 d = dp * (1 - w) + dq * w;
            float h = hp * (1 - w) + hq * w;

            const map<double, vec3>::const_iterator dl = colorstopuv.lower_bound(w);
            const map<double, vec3>::const_iterator dr = next(dl, -1);

            vec3 cn = (dr->second - dl->second) * (w - dl->first) / (dr->first - dl->first) + dl->second;

            vec3 cd = (!isShowAfter) ? co : cn;
            vec3 cu = (!isShowBefore) ? cn : co;

            double x = time / double(frame_cnt - 1) - .5;
            double y = h - .5;

            glColor4f(cu[0], cu[1], cu[2], 0.4f);
            glBegin(GL_POLYGON);
            for(int k = 0; k <= 16; k++)
            {
                glVertex3f(x + cos(k / 16. * M_PI) * ha, y + sin(k / 16. * M_PI) * ha * aspect , 0.2);
            }
            glEnd();

            glColor4f(cd[0], cd[1], cd[2], 0.4f);
            glBegin(GL_POLYGON);
            for(int k = -16; k <= 0; k++)
            {
                glVertex3f(x + cos(k / 16. * M_PI) * ha, y + sin(k / 16. * M_PI) * ha * aspect , 0.2);
            }
            glEnd();

            // draw arrow
            if(abs(cu[0] - cd[0]) + abs(cu[1] - cd[1]) + abs(cu[2] - cd[2]) > 5e-2)
            {
                /*
                if(u[0] + u[1] + u[2] < 1.5f)
                    glColor4f(1.f, 1.f, 1.f, 1.f);
                else
                    glColor4f(0.f, 0.f, 0.f, 1.f);
                */

                if(cu[0] + cu[1] + cu[2] + cd[0] + cd[1] + cd[2] < 3.f)
                    glColor4f(1.f, 1.f, 1.f, 0.4f);
                else
                    glColor4f(0.f, 0.f, 0.f, 0.4f);

                glLineWidth(1.f);
                glBegin(GL_LINES);
                glVertex3f(x, y, 0.1);
                glVertex3f(x, y + ha * 0.55 * aspect, 0.1);
                glEnd();

                /*
                if(d[0] + d[1] + d[2] < 1.5f)
                    glColor4f(1.f, 1.f, 1.f, 1.f);
                else
                    glColor4f(0.f, 0.f, 0.f, 1.f);
                */
                glLineWidth(1.f);
                glBegin(GL_LINES);
                glVertex3f(x, y, 0.1);
                glVertex3f(x, y - ha * 0.65 * aspect, 0.1);

                glVertex3f(x - 0.45 * ha, y - ha * 0.05 * aspect, 0.1);
                glVertex3f(x, y - ha * 0.65 * aspect, 0.1);

                glVertex3f(x + 0.45 * ha, y - ha * 0.05 * aspect, 0.1);
                glVertex3f(x, y - ha * 0.65 * aspect, 0.1);
                glEnd();
            }
        }
        ha /= 0.8;
    }

    ha /= 0.8;

    for (int i = 0; i < up_vertex.size(); i++)
    {
        const vec4 &u = up_vertex[i];
        const vec4 &d = down_vertex[i];

        //float x = op[3] / float(frame_cnt - 1) - 0.5;
        float x = u[3] - 0.5;
        float y = vertex_palette_position[i] - 0.5;

        float scale;

        if (selected_vid == i)
        {
            // scale = 1.5;
            // glColor3f(0.8, 0, 0);
            // glRectf(x - ha_selected, y - ha_selected*aspect, x + ha_selected, y + ha_selected*aspect);
            scale = 1.0;
        }
        else
        {
            scale = 1;
        }

        glColor4f(u[0], u[1], u[2], 1.f);
        glBegin(GL_POLYGON);
        for(int k = 0; k <= 16; k++)
        {
            glVertex3f(x + cos(k / 16. * M_PI) * ha *  scale, y + sin(k / 16. * M_PI) * ha * scale * aspect , 0.);
        }
        glEnd();
        if(selected_vid == i)
        {
            for(int k = 0; k < 16; k++)
            {
                // if((k & 3) >= 2) continue;
                glBegin(GL_POLYGON);
                glVertex3f(x + cos(k / 16. * M_PI) * ha *  scale * 1.25, y + sin(k / 16. * M_PI) * ha * scale * aspect * 1.25 , 0.);
                glVertex3f(x + cos(k / 16. * M_PI) * ha *  scale * 1.50, y + sin(k / 16. * M_PI) * ha * scale * aspect * 1.50 , 0.);
                k++;
                glVertex3f(x + cos(k / 16. * M_PI) * ha *  scale * 1.50, y + sin(k / 16. * M_PI) * ha * scale * aspect * 1.50 , 0.);
                glVertex3f(x + cos(k / 16. * M_PI) * ha *  scale * 1.25, y + sin(k / 16. * M_PI) * ha * scale * aspect * 1.25 , 0.);
                k--;
                glEnd();
            }
        }

        glColor4f(d[0], d[1], d[2], 1.f);
        glBegin(GL_POLYGON);
        for(int k = -16; k <= 0; k++)
        {
            glVertex3f(x + cos(k / 16. * M_PI) * ha * scale, y + sin(k / 16. * M_PI) * ha * scale * aspect , 0.);
        }
        glEnd();
        if(selected_vid == i)
        {
            for(int k = -16; k < 0; k++)
            {
                // if((k & 3) >= 2) continue;

                glBegin(GL_POLYGON);
                glVertex3f(x + cos(k / 16. * M_PI) * ha *  scale * 1.25, y + sin(k / 16. * M_PI) * ha * scale * aspect * 1.25 , 0.);
                glVertex3f(x + cos(k / 16. * M_PI) * ha *  scale * 1.50, y + sin(k / 16. * M_PI) * ha * scale * aspect * 1.50 , 0.);
                k++;
                glVertex3f(x + cos(k / 16. * M_PI) * ha *  scale * 1.50, y + sin(k / 16. * M_PI) * ha * scale * aspect * 1.50 , 0.);
                glVertex3f(x + cos(k / 16. * M_PI) * ha *  scale * 1.25, y + sin(k / 16. * M_PI) * ha * scale * aspect * 1.25 , 0.);
                k--;
                glEnd();
            }
        }

        if(abs(u[0] - d[0]) + abs(u[1] - d[1]) + abs(u[2] - d[2]) > 5e-2)
        {
            if(u[0] + u[1] + u[2] + d[0] + d[1] + d[2] < 3.f)
                glColor4f(1.f, 1.f, 1.f, 1.f);
            else
                glColor4f(0.f, 0.f, 0.f, 1.f);

            glLineWidth(1.f);
            glBegin(GL_LINES);
            glVertex3f(x, y, -.5);
            glVertex3f(x, y + ha * scale * 0.55 * aspect, -.5);
            glEnd();

            glLineWidth(1.f);
            glBegin(GL_LINES);
            glVertex3f(x, y, -.5);
            glVertex3f(x, y - ha * scale * 0.65 * aspect, -.5);

            glVertex3f(x - 0.45 * ha * scale, y - ha * scale * 0.05 * aspect, -.5);
            glVertex3f(x, y - ha * scale * 0.65 * aspect, -.5);

            glVertex3f(x + 0.45 * ha * scale, y - ha * 0.05 * aspect, -.5);
            glVertex3f(x, y - ha * scale * 0.65 * aspect, -.5);
            glEnd();
        }
    }
}


void PaletteViewWidget::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton) {
        //1. 坐标系变换，将屏幕坐标系 -> opengl坐标系
        int w = width(), h = height();
        double half_w = w / 2.0, half_h = h / 2.0;
        double new_x = (ev->x() - half_w) / half_w / scale;
        double new_y = -(ev->y() - half_h) / half_h / scale;

        last_click_y_pos = new_y;

        int frame_cnt = data->getFrmCnt();
        vector<vec4> palette_vertices = data->getCurVideoPalette().vertices;
        const vector<int2> &palette_edges = data->getOriVideoPalette().edges;
        const double *vertex_palette_position = data->getCurVideoPalette_palette_position();

        selected_vid = -1;
        selected_eid = -1;

        //2. 找到离鼠标点击最近的点
        double th = 1.0 / min(half_h, half_w) * 10;
        for (int i = 0; i < palette_vertices.size() && selected_vid == -1 && selected_eid == -1; i++)
        {
            vec4 p = palette_vertices[i];
            double hp = vertex_palette_position[i];
            double tp = p[3];

            //double x = tp / double(frame_cnt - 1) - 0.5;
            double x = tp  - 0.5;
            double y = hp - 0.5;

            double dis = mysqrt((new_x - x)*(new_x - x) + (new_y - y)*(new_y - y));
            if (dis < th) {
                selected_vid = i;

                double r = p[0], g = p[1], b = p[2];
                int R = r * 255, G = g * 255, B = b * 255;
                QColor c;
                c.setRed(R);
                c.setGreen(G);
                c.setBlue(B);
                Q_EMIT setColor(c);
                update();
            }
        }

        for(int i = 0; i < palette_edges.size() && selected_vid == -1 && selected_eid == -1; i++)
        {
            int uid = palette_edges[i][0];
            int vid = palette_edges[i][1];

            if (palette_vertices[uid][3] > palette_vertices[vid][3]) std::swap(uid, vid);

            if(!(palette_vertices[uid][3] < new_x + 0.5 && new_x + 0.5 < palette_vertices[vid][3])) continue;

            double w = (new_x + 0.5 - palette_vertices[uid][3]) / (palette_vertices[vid][3] - palette_vertices[uid][3]);

            double h = vertex_palette_position[uid] * (1 - w) + vertex_palette_position[vid] * w;

            double dis = fabs(new_y + 0.5 - h);
            if (dis < th)
            {
#ifndef USER_STUDY
                selected_eid = i;
                update();
#endif
            }
        }

        // if(selected_vid == -1 && selected_eid == -1)
        {
            int newTime = (new_x + .5) * double(frame_cnt - 1) + .5;
            if (newTime < 0) newTime = 0;
            if (newTime > frame_cnt - 1) newTime = frame_cnt - 1;
            emit timeChanged(newTime);
        }
    }
    else if (ev->button() == Qt::RightButton) {
        pMenu->exec(cursor().pos());
    }
}

/*
void PaletteViewWidget::keyPressEvent(QKeyEvent *event)
{

}
*/

bool PaletteViewWidget::eventFilter(QObject *target, QEvent *e)
{
    Q_UNUSED(target);

    bool handled = false;
    if(e->type() == QEvent::KeyPress)
    {
        qDebug() << "PaletteViewWidget::eventFilter";

        QKeyEvent *keyEvent = (QKeyEvent *)e;

        switch(keyEvent->key())
        {
        case Qt::Key_F1:
            heightAdsorb = 1;
            break;
        case Qt::Key_F2:
            heightAdsorb = 2;
            break;
        case Qt::Key_F3:
            heightAdsorb = 3;
            break;
        case Qt::Key_F4:
            heightAdsorb = 4;
            break;
        case Qt::Key_F5:
            heightAdsorb = 5;
            break;
        case Qt::Key_F6:
            heightAdsorb = 6;
            break;
        case Qt::Key_F7:
            heightAdsorb = 7;
            break;
        case Qt::Key_F8:
            heightAdsorb = 8;
            break;
        case Qt::Key_F9:
            heightAdsorb = 9;
            break;
        }
    }
    return handled;
}

void PaletteViewWidget::mouseMoveEvent(QMouseEvent *ev)
{
    int w = width(), h = height();
    float half_w = w * .5f, half_h = h * .5f;
    float new_x = (ev->x() - half_w) / half_w / scale;
    float new_y = -(ev->y() - half_h) / half_h / scale;
    int frame_cnt = data->getFrmCnt();

    if (new_y >  .5f) new_y = .5f;
    if (new_y < -.5f) new_y = -.5f;

    if (selected_vid >= 0)
    {
        if (fabs(new_y - last_click_y_pos) > 1e-2)
        {
            float new_height = 1e5f;

            if(QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)
            {
#ifndef USER_STUDY
                for(int p = heightAdsorb; p <= heightAdsorb; p++)
#else
                for(int p = 1; p <= 7; p++)
#endif
                    for(int k = 0; k < p; k++)
                    {
                        if(fabs(new_y +.5f - new_height) > fabs(new_y + .5f - (k + 0.5f) / (double) p))
                             new_height = (k + 0.5f) / (double)p;
                    }
            }
            else
                new_height = new_y + .5f;
#ifndef USER_STUDY
            data->setCurVideoPalette_palette_position(selected_vid, new_height);
#endif
        }
    }
    else if(selected_eid >= 0)
    {

    }
    // else if(selected_vid == -1 && selected_eid == -1)
    {
        int newTime = (new_x + .5) * double(frame_cnt - 1) + .5;
        if (newTime < 0) newTime = 0;
        if (newTime > frame_cnt - 1) newTime = frame_cnt - 1;
        emit timeChanged(newTime);
    }
}

void PaletteViewWidget::resetVertex() {
    if (selected_vid != -1) {
        data->resetVertex(selected_vid);
        data->Recolor(time);
        emit update();
    }
}
void PaletteViewWidget::resetAllVertex() {
    data->resetAllVertex();
    data->Recolor(time);
    emit update();
}

void PaletteViewWidget::setShowPalette(bool before, bool after)
{
    isShowBefore = before;
    isShowAfter = after;
    emit update();
}
