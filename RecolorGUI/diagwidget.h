#ifndef DIAGWIDGET_H
#define DIAGWIDGET_H

#include"opengl3dwidget.h"

class DiagWidget : public OpenGL3DWidget
{
public:
    explicit DiagWidget(QWidget *parent = 0);
    void wheelEvent(QWheelEvent *event) override;

public slots:
    void setWindow(double logWindow);

protected:
    void paintGL() Q_DECL_OVERRIDE;
    double logWindow = -.9;
};

#endif // DIAGWIDGET_H
