#ifndef RGBWIDGET_H
#define RGBWIDGET_H

#include<opengl3dwidget.h>
#include"data.h"
#include<QSharedMemory>

class RGBWidget : public OpenGL3DWidget
{
public:
    RGBWidget(bool original);
protected:
    void paintGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h);

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
private:
    bool original = false;

    static double _rotate_x;
    static double _rotate_y;
    static double _rotate_z;

    QSharedMemory sharedMemory;
};

#endif // RGBWIDGET_H
