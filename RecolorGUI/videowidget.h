#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QString>
#include "data.h"

class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(bool _isAfter, QWidget *parent = nullptr);

    void setData(Data *value);

protected:
    virtual void paintEvent(QPaintEvent *event);
signals:

public slots:
    void setTime(int t);
    void update();

protected:
    Data *data = nullptr;
    int time = 0;

private:
    bool isAfter;
};

#endif // VIDEOWIDGET_H
