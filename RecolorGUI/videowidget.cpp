#include "videowidget.h"
#include <QFile>
#include <QIODevice>

#include <QDebug>
#include <QPainter>

// Widget shows a video

VideoWidget::VideoWidget(bool _isAfter, QWidget *parent) : QWidget(parent), isAfter(_isAfter)
{
}

void VideoWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    if(data == nullptr) return;

    int frame_cnt = data->getFrmCnt();
    int width = data->getFrmWidth();
    int height = data->getFrmHeight();
    int depth = data->getFrmDepth();
    double *video = data->getVideo(isAfter);



    QPainter painter(this);
    // qDebug() << "VideoWidget::paintEvent```````" << width << height;
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    uchar *data = new uchar[width * height * depth];

    for(int i = 0; i < width * height * depth; i++)
    {
        int x = static_cast<int>(video[time * width * height * depth + i] * 255);
        if(x > 255) x = 255;
        if(x < 0) x = 0;

        data[i] = static_cast<uchar>(x);
    }

    //QImage im(data, height, width, height * 3, QImage::Format_RGB888);
	QImage im(data, width, height, width * 3, QImage::Format_RGB888);

    painter.setPen(QPen(Qt::red,3));

#ifdef INPAPER
    painter.drawImage(QRectF(0, 0, width * 0.65, height * 0.65), im);
#elif USER_STUDY
    painter.drawImage(QRectF(0, 0, width * 0.86614173, height * 0.86614173), im);
#else
    painter.drawImage(QRectF(0, 0, width, height), im);
#endif

    // painter.drawEllipse(2,2,10,20);

    painter.end();
    delete[] data;
}

void VideoWidget::setTime(int t)
{
    time = t;

    update();
}

void VideoWidget::update()
{
    QWidget::update();

    int w = data->getFrmWidth();
    int h = data->getFrmHeight();

    //setMaximumSize(h, w);
    //setMinimumSize(h, w);

#ifdef INPAPER
    setMaximumSize(w*0.65, h*0.65);
    setMinimumSize(w*0.65, h*0.65);
#elif USER_STUDY
    setMaximumSize(w*0.86614173, h*0.86614173);
    setMinimumSize(w*0.86614173, h*0.86614173);
#else
    setMaximumSize(w, h);
    setMinimumSize(w, h);
#endif

}

void VideoWidget::setData(Data *value)
{
    data = value;

    connect(value, &Data::updated, this, &VideoWidget::update);
}

