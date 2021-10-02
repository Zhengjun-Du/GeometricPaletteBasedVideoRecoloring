#ifndef ABTWIDGET_H
#define ABTWIDGET_H

#include<QWindow>
#include<QOpenGLFunctions>
#include<QOpenGLPaintDevice>

#include <QOpenGLWidget>
#include <QOpenGLFunctions>

#include <QGLFramebufferObjectFormat>

#include"opengl3dwidget.h"
class ABTWidget : public OpenGL3DWidget
{
public:
    explicit ABTWidget(QWidget *parent = 0);
    void wheelEvent(QWheelEvent *event) override;

public slots:
    void setWindow(double logWindow);

protected:
    void paintGL() Q_DECL_OVERRIDE;
    double logWindow = -.9;
};


#endif // ABTWIDGET_H
