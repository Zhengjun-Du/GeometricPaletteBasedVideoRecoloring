#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <data.h>
#include <QSlider>
#include <QProxyStyle>
#include <QStyleOption>
#include <QPainter>


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *target, QEvent *e) override;

private:
	QDockWidget *videoBeforeDockWidget = nullptr;
	QDockWidget *videoAfterDockWidget = nullptr;
    QDockWidget *dockRBGCurrentWidget = nullptr;
    QDockWidget *dockRBGOriginalWidget = nullptr;

    Data *data = nullptr;
    QSlider *slider = nullptr;
    QSlider *mergeStepSlider = nullptr;

	void openFile(bool merge);
	void importWeights();
	void exportWeights();
	void importPalette();
	void exportPalette();
    void calculateWeights(bool mvc_, bool rgbxy_, bool lbc_);
    void exportVideo();
    void exportTikZ();

    void openUserStudy();

    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *e);

    // QString title = "Video Recoloring Tool (Build " __DATE__ " " __TIME__ ")";
    // QString title = "Video Recoloring Tool";
    QString title = "Palette-Based Video Recoloring";

    bool isPlaying = false;
    double preview = 0.25;
};

class IconnedDockStyle: public QProxyStyle
{
    Q_OBJECT
    QIcon icon_;
public:
    IconnedDockStyle(const QIcon& icon,  QStyle* style = 0)
        : QProxyStyle(style)
        , icon_(icon)
    {}

    virtual ~IconnedDockStyle()
    {}

    virtual void drawControl(ControlElement element, const QStyleOption* option,
        QPainter* painter, const QWidget* widget = 0) const
    {
        if(element == QStyle::CE_DockWidgetTitle)
        {
            //width of the icon
            int width = pixelMetric(QStyle::PM_ToolBarIconSize) / 2;
            //margin of title from frame
            int margin = baseStyle()->pixelMetric(QStyle::PM_DockWidgetTitleMargin);

            QPoint icon_point(margin + option->rect.left(), margin + option->rect.center().y() - width/2);

            painter->drawPixmap(icon_point, icon_.pixmap(width, width));

            const_cast<QStyleOption*>(option)->rect = option->rect.adjusted(width/2, 0, 0, 0);
        }
        baseStyle()->drawControl(element, option, painter, widget);
    }
};

#endif // MAINWINDOW_H
