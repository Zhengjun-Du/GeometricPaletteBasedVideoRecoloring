#ifndef PALETTEVIEWWIDGET_H
#define PALETTEVIEWWIDGET_H

#include<openglwidget.h>
#include <QMenu>

class PaletteViewWidget : public OpenGLWidget
{
Q_OBJECT
public:
    explicit PaletteViewWidget(QWidget *parent = 0);

protected:
    void paintGL() Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    // void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *target, QEvent *e) override;

public Q_SLOTS:
	void getColor(QColor c);
	void setTime(int t_);

	void resetVertex();
	void resetAllVertex();
	void setShowPalette(bool before, bool after);
	
Q_SIGNALS:
	void setColor(QColor c);
	void timeChanged(int);

private:
    bool blink = false;
    double scale = 1.8;
    int selected_vid = -1;
    int selected_eid = -1;
	double aspect = 1.0;
	int time = 0;

    int heightAdsorb = 1;

	QMenu *pMenu;

	double last_click_y_pos;
	bool isShowBefore = true;
	bool isShowAfter = true;
};

#endif // PALETTEVIEWWIDGET_H
