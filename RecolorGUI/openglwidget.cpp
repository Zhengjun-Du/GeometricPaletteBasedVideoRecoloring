#include "openglwidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QDebug>

// basic OpenGL 2D widget

OpenGLWidget::OpenGLWidget(QWidget *parent) : QOpenGLWidget(parent)
{
}

void OpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    //  glEnable(GL_DEPTH_TEST);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_BLEND); //Enable blending.
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //Set blending function.

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glClearDepth(1.0f);
    //  Borrar pantalla y Z-buffer
    glClearColor(.5f, .5f, .5f, 0.0f);
}

void OpenGLWidget::resizeGL(int w, int h)
{
    qDebug() << "resizeGL" << w << h;
    int m = w < h ? w : h;
    glViewport(0, 0, m, m);
    update();
}

void OpenGLWidget::paintGL()
{
    QOpenGLWidget::paintGL();


    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glShadeModel(GL_SMOOTH);
    // Resetear transformaciones
    glLoadIdentity();

}

void OpenGLWidget::setData(Data *value)
{
    data = value;
    connect(value, &Data::updated, this, &OpenGLWidget::update);
}

void OpenGLWidget::update()
{
    QOpenGLWidget::update();
}

void OpenGLWidget::setTime(int t)
{
    // qDebug() << t;

    time = t;
    update();
}

void OpenGLWidget::setShowVideoData(bool _showVideoData)
{
    showVideoData = _showVideoData;
    update();
}

void OpenGLWidget::setPreview(int percentage)
{
    preview = percentage / 1000.;
    update();
}

/*
void OpenGLWidget::graficarLineas()
{
    glBegin(GL_LINES);
    glColor3f(1,0,0);
    glVertex3f(0,0,0);
    glVertex3f(20,0,0);

    glColor3f(1,1,0);
    glVertex3f(0,0,0);
    glVertex3f(0,20,0);

    glColor3f(0,1,1);
    glVertex3f(0,0,0);
    glVertex3f(0,0,20);
    glEnd();
}
*/


