#ifndef OPENGL3DWIDGET_H
#define OPENGL3DWIDGET_H

#include"openglwidget.h"

class OpenGL3DWidget : public OpenGLWidget
{
public:
    OpenGL3DWidget(QWidget *parent = 0);
protected:
    void paintGL() Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
protected:
    double rotate_x;
    double rotate_y;
    double rotate_z;
private:
    QPoint lastPos;
};

#endif // OPENGL3DWIDGET_H
