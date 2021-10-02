#include "opengl3dwidget.h"
#include<QMouseEvent>
#include<QDebug>

// basic OpenGL 3D Widget

OpenGL3DWidget::OpenGL3DWidget(QWidget *parent) : OpenGLWidget(parent), rotate_x(-15), rotate_y(-70), rotate_z(0), lastPos(0, 0)
{
    lastPos.setX(0);
    lastPos.setY(0);
}

void OpenGL3DWidget::paintGL()
{
    OpenGLWidget::paintGL();

    // Otras transformaciones
    // glTranslatef( 0.1, 0.0, 0.0 );      // No incluido
    // glRotatef( 180, 0.0, 1.0, 0.0 );    // No incluido

    // Rotar cuando el usuario cambie “rotate_x” y “rotate_y”
    glRotatef( rotate_x, 1.0, 0.0, 0.0 );
    glRotatef( rotate_y, 0.0, 1.0, 0.0 );
    glRotatef( rotate_z, 0.0, 0.0, 1.0 );
    glScalef( 1.2, 1.2, -1.2 );

}

void OpenGL3DWidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

void OpenGL3DWidget::mouseMoveEvent(QMouseEvent *event)
{
    QOpenGLWidget::mouseMoveEvent(event);

    if (event->buttons() & Qt::LeftButton)
    {
        int dx = event->x() - lastPos.x();
        int dy = event->y() - lastPos.y();

        // qDebug() << rotate_x << rotate_y << event->x() << event->y() << lastPos.x() << lastPos.y();

        lastPos = event->pos();
        rotate_x -= dy;
        rotate_y -= dx;
        qDebug() << "mouseMoveEvent" << rotate_x << rotate_y;

        update();
    }
}


