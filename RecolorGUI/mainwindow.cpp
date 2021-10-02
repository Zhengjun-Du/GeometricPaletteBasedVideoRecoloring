#include<QWidget>
#include<QPushButton>
#include<QVBoxLayout>
#include<QSlider>
#include<QLabel>
#include<Q3DSurface>
#include<QtDataVisualization>
#include<QDockWidget>
#include<QTextEdit>
#include<QListWidget>
#include<QFileDialog>
#include<QCheckBox>
#include<QSplitter>
#include<QSizePolicy>
#include<QProgressDialog>
#include <QGroupBox>
#include <QButtonGroup>
#include <QRadioButton>
#include <QMessageBox>
#include <QAction>
#include <QToolBar>
#include <QStyle>
#include <QApplication>
#include <QProxyStyle>
#include <QShortcut>

#include "data.h"

#include "mainwindow.h"
#include "videowidget.h"
#include "openglwidget.h"
#include "abtwidget.h"
#include "rgbwidget.h"
#include "diagwidget.h"
#include "paletteviewwidget.h"
#ifdef _WIN32
#  include "Qt-Color-Widgets\include\QtColorWidgets\color_dialog.hpp"
#else
#  include "QtColorWidgets/color_dialog.hpp"
#endif

// Setup UI

#include <QColorDialog>

using namespace color_widgets;

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent)
{
    this->setWindowIcon(QIcon(QPixmap(":/icons/color-swatches.png")));
    this->setWindowFilePath("/dev/null");
    this->setWindowTitle(title);
#ifdef INPAPER
    this->setGeometry(60, 110, 10, 10);
#else
    this->setGeometry(60, 110, 650, 700);
#endif
	this->data = new Data();

	//show original video and recolored video====================================================================
	VideoWidget *recolored_video = new VideoWidget(true);
	recolored_video->setData(data);
	VideoWidget *original_video = new VideoWidget(false);
	original_video->setData(data);

	videoBeforeDockWidget = new QDockWidget();
	videoBeforeDockWidget->setWidget(original_video);
	videoBeforeDockWidget->setWindowTitle("Original Video");
    videoBeforeDockWidget->setWindowIcon(QIcon(QPixmap(":/icons/film.png")));
    videoBeforeDockWidget->setWindowFilePath("/dev/null");
	addDockWidget(Qt::TopDockWidgetArea, videoBeforeDockWidget);
	videoBeforeDockWidget->setFloating(true);
	videoBeforeDockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    videoBeforeDockWidget->setGeometry(730, 120, 550, 300);
#ifndef USER_STUDY
	videoBeforeDockWidget->hide();
#endif

	videoAfterDockWidget = new QDockWidget();
	videoAfterDockWidget->setWidget(recolored_video);
	videoAfterDockWidget->setWindowTitle("Recolored Video");
    videoAfterDockWidget->setWindowIcon(QIcon(QPixmap(":/icons/film--pencil.png")));
    videoAfterDockWidget->setWindowFilePath("/dev/null");
	videoAfterDockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
	addDockWidget(Qt::TopDockWidgetArea, videoAfterDockWidget);
	videoAfterDockWidget->setFloating(true);
    videoAfterDockWidget->setGeometry(1300, 120, 550, 300);
#ifndef USER_STUDY
	videoAfterDockWidget->hide();
#endif
	//show original video and recolored video====================================================================

#ifdef USER_STUDY

    QAction *openAction = new QAction(QIcon(":/icons/blue-folder-horizontal-open.png"), tr("Open"), this); // Video && Palette
    QAction *saveAction = new QAction(QIcon(":/icons/disk.png"), tr("Save Palette"), this);
    QAction *exportAction = new QAction(QIcon(":/icons/film--arrow.png"), tr("Export Video"), this);
    QAction *resetAction = new QAction(QIcon(":/icons/eraser.png"), tr("Reset Selected"), this);
    QAction *discardAction = new QAction(QIcon(":/icons/eraser--exclamation.png"), tr("Reset All"), this);
    QAction *playAction = new QAction(QIcon(":/icons/control.png"), tr("Play"), this);
    // QAction *pauseAction = new QAction(QIcon(":/icons/control-pause.png"), tr("Pause"), this);
#ifdef INPAPER
    for(auto i : {openAction, saveAction, exportAction, resetAction, discardAction, playAction})
    {
        QFont f = i->font();
        f.setBold(true);
        f.setPointSize(11);
        i->setFont(f);
    }
#endif

    connect(new QShortcut(Qt::Key_Return, this), &QShortcut::activated, playAction, &QAction::trigger);
    connect(new QShortcut(Qt::Key_Enter, this), &QShortcut::activated, playAction, &QAction::trigger);
    connect(new QShortcut(Qt::Key_Space, this), &QShortcut::activated, playAction, &QAction::trigger);
    connect(new QShortcut(Qt::Key_Any, this), &QShortcut::activated, playAction, &QAction::trigger);

    QToolBar *toolBar = addToolBar(tr("File"));
    toolBar->setIconSize(QSize(16, 16));
    toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBar->addAction(openAction);
    toolBar->addAction(saveAction);
    toolBar->addAction(exportAction);

    toolBar = addToolBar(tr("Edit"));
    toolBar->setIconSize(QSize(16, 16));
    toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBar->addAction(resetAction);
    toolBar->addAction(discardAction);

    toolBar = addToolBar(tr("Timeline Control"));
    toolBar->setIconSize(QSize(16, 16));
    toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBar->addAction(playAction);
    // toolBar->addAction(pauseAction);
#endif

	//0.buttons in the top=======================================================================================
	QWidget *mainWidget = new QWidget();
	QVBoxLayout *mainLayout = new QVBoxLayout();

	QHBoxLayout *firstRowLayout = new QHBoxLayout();
	QPushButton *openVideoAndPaletteButton = new QPushButton("Open Video and Palette");
	QPushButton *importWeightsButton = new QPushButton("Import Weights");
	importWeightsButton->hide();
	QPushButton *importPaletteButton = new QPushButton("Import Palette");
	QPushButton *rgbButton = new QPushButton("RGB View");
    QPushButton *exportTikZBtn = new QPushButton("Export TikZ Code");
	exportTikZBtn->hide();

	firstRowLayout->addWidget(openVideoAndPaletteButton);
	firstRowLayout->addWidget(importPaletteButton);
	firstRowLayout->addWidget(importWeightsButton);
	firstRowLayout->addWidget(rgbButton);
    firstRowLayout->addWidget(exportTikZBtn);
#ifndef USER_STUDY
	mainLayout->addLayout(firstRowLayout);
#endif
	//0.buttons in the top=======================================================================================

	//1.import and export========================================================================================
	QRadioButton *mvc_radio = new QRadioButton("MVC");
	mvc_radio->setChecked(1);
	mvc_radio->hide();
	QRadioButton *rgbxy_radio = new QRadioButton("RGBXY"); rgbxy_radio->hide();
	QRadioButton *lbc_radio = new QRadioButton("LBC"); lbc_radio->hide();
	QButtonGroup *group = new QButtonGroup(this);
	group->addButton(mvc_radio);
	group->addButton(rgbxy_radio);
	group->addButton(lbc_radio);

	QPushButton *calWeightsBtn = new QPushButton("Calculate Weights");
	QPushButton *saveWeightsBtn = new QPushButton("Save Weights");
	saveWeightsBtn->hide();
    QPushButton *savePaletteBtn = new QPushButton("Save Palette");
    QPushButton *exportVideoeBtn = new QPushButton("Export Video");
	QLabel *weightsLabel = new QLabel("Weights:"); weightsLabel->hide();

	QHBoxLayout *showImporExportLayout = new QHBoxLayout();
	showImporExportLayout->addWidget(weightsLabel);
	showImporExportLayout->addWidget(mvc_radio);
	showImporExportLayout->addWidget(rgbxy_radio);
	showImporExportLayout->addWidget(lbc_radio);
	showImporExportLayout->addWidget(calWeightsBtn);
	showImporExportLayout->addWidget(saveWeightsBtn);
    showImporExportLayout->addWidget(savePaletteBtn);
    showImporExportLayout->addWidget(exportVideoeBtn);
	//1.import and export========================================================================================

	//2.palette + time setting===================================================================================
	/*
	QHBoxLayout *paletteThresholdLayout = new QHBoxLayout();
	QSlider *paletteThresholdSlider = new QSlider(Qt::Horizontal);
	QLabel *paletteThresholdLabel = new QLabel();
	paletteThresholdLayout->addWidget(new QLabel("Palette Threshold:"));
	paletteThresholdLayout->addWidget(paletteThresholdSlider);
	paletteThresholdLayout->addWidget(paletteThresholdLabel);
	paletteThresholdSlider->setMinimum(0);
	paletteThresholdSlider->setMaximum(3000);

	connect(paletteThresholdSlider, &QSlider::valueChanged,
	[=](const int &newValue){paletteThresholdLabel->setText(QString::number(newValue / 1000., 'f', 3));} );
	connect(paletteThresholdSlider, &QSlider::sliderReleased,
	[=](){data->setThreshold(paletteThresholdSlider->value() / 1000.);});

	paletteThresholdSlider->setValue(200);
	*/
	QHBoxLayout *paletteTimeSettingLayout = new QHBoxLayout();
	QButtonGroup *buttonPaletteTimeSettingGroup = new QButtonGroup();
	QRadioButton *buttonPaleeteTimeBeforeOnly = new QRadioButton("Original Palette Only");
	QRadioButton *buttonPaleeteTimeAfterOnly = new QRadioButton("Recolred Palette Only");
	QRadioButton *buttonPaleeteTimeBoth = new QRadioButton("Both");
	buttonPaletteTimeSettingGroup->addButton(buttonPaleeteTimeBeforeOnly);
	buttonPaletteTimeSettingGroup->addButton(buttonPaleeteTimeAfterOnly);
	buttonPaletteTimeSettingGroup->addButton(buttonPaleeteTimeBoth);
	paletteTimeSettingLayout->addWidget(new QLabel("Palette-Time View Setting:"));
	paletteTimeSettingLayout->addWidget(buttonPaleeteTimeBeforeOnly);
	paletteTimeSettingLayout->addWidget(buttonPaleeteTimeAfterOnly);
	paletteTimeSettingLayout->addWidget(buttonPaleeteTimeBoth);
	buttonPaleeteTimeBoth->click();

	//2.palette + time setting===================================================================================

	//3.RGB view setting=========================================================================================
	QHBoxLayout *showVideoDataLayout = new QHBoxLayout();
	QCheckBox *showVideoDataCheck = new QCheckBox();
	QLabel *showVideoDataLabel = new QLabel("25.0%");
	QSlider *videoPreviewSlider = new QSlider(Qt::Horizontal);

	showVideoDataCheck->setCheckState(Qt::Checked);
	showVideoDataCheck->setText("Visualize Video Data Points");

	videoPreviewSlider->setMinimum(0);
	videoPreviewSlider->setMaximum(1000);
	videoPreviewSlider->setValue(250);
	showVideoDataLabel->setBuddy(videoPreviewSlider);
	showVideoDataLabel->setWordWrap(false);

	connect(videoPreviewSlider, &QSlider::valueChanged,
		[=](const int &newValue) {showVideoDataLabel->setText(QString::number(newValue / 10., 'f', 1) + "%"); });

	showVideoDataLayout->addWidget(showVideoDataCheck);
	showVideoDataLayout->addWidget(videoPreviewSlider);
	showVideoDataLayout->addWidget(showVideoDataLabel);
	//layout->addLayout(showVideoDataLayout);
	//layout->setStretchFactor(showVideoDataLayout, 1);
	//3.RGB view setting=========================================================================================

	//4.video progress===========================================================================================
	QHBoxLayout *timeWindowLayout = new QHBoxLayout();
	QSlider *timeWindowSlider = new QSlider(Qt::Horizontal);
	timeWindowSlider->setMinimum(-3000);
	timeWindowSlider->setMaximum(10000);

	QHBoxLayout *timeBarLayout = new QHBoxLayout();
	QLabel *timeBarTitleLabel = new QLabel("Video Progress:");
	QLabel *timeBarLabel = new QLabel("0");

	slider = new QSlider(Qt::Horizontal);
	slider->setTickInterval(1);

	QPushButton *autoPlayButton = new QPushButton("Play ");

	QTimer *autoPlayTimer = new QTimer();

	timeBarLayout->addWidget(timeBarTitleLabel);
	timeBarLayout->addWidget(slider);
	timeBarLayout->addWidget(timeBarLabel);
	timeBarLayout->addWidget(autoPlayButton);

	connect(slider, &QSlider::valueChanged,
		[=](const int &newValue) {timeBarLabel->setText(QString::number(newValue)); });

    autoPlayTimer->setInterval(40);
	connect(autoPlayTimer, &QTimer::timeout, [=]() {
		int pos = slider->value();
		if (pos >= data->getFrmCnt() - 1)
		{
#ifndef USER_STUDY
            autoPlayButton->click();
#else
            playAction->trigger();
#endif
		}
		else
		{
			slider->setValue(pos + 1);
		}
	});

#ifndef USER_STUDY
	connect(autoPlayButton, &QPushButton::clicked, [=]() {
		isPlaying = !isPlaying;
		if (isPlaying)
		{
			int pos = slider->value();
			if (pos >= data->getFrmCnt() - 1)
			{
				slider->setValue(0);
			}
			autoPlayButton->setText("Pause");
			autoPlayTimer->start();
		}
		else
		{
			autoPlayButton->setText("Play ");
			autoPlayTimer->stop();
		}
	});
#else
    connect(playAction, &QAction::triggered, [=]() {
        // qDebug() << "test";
        isPlaying = !isPlaying;
        if (isPlaying)
        {
            int pos = slider->value();
            if (pos >= data->getFrmCnt() - 1)
            {
                slider->setValue(0);
            }
            playAction->setText("Pause");
            playAction->setIcon(QIcon(":/icons/control-pause.png"));
            autoPlayTimer->start();
        }
        else
        {
            playAction->setText("Play");
            playAction->setIcon(QIcon(":/icons/control.png"));
            autoPlayTimer->stop();
        }
    });
#endif

	//4.video progress===========================================================================================

    QGroupBox *groupBox = new QGroupBox(tr("Parameters setting"));
	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->addLayout(showImporExportLayout);
	// vbox->addLayout(paletteThresholdLayout);
	vbox->addLayout(paletteTimeSettingLayout);
	vbox->addLayout(showVideoDataLayout);
	vbox->addLayout(timeBarLayout);
	groupBox->setLayout(vbox);


	//RGB view===================================================================================================

    dockRBGOriginalWidget = new QDockWidget();
    RGBWidget *rgbOriginalWidget = new RGBWidget(true);
    rgbOriginalWidget->setMinimumSize(100, 100);
    rgbOriginalWidget->setData(data);

    dockRBGOriginalWidget->setWidget(rgbOriginalWidget);
    dockRBGOriginalWidget->setWindowTitle("Original RGB");
    dockRBGOriginalWidget->setWindowIcon(QIcon(QPixmap(":/icons/ice.png")));
    dockRBGOriginalWidget->setWindowFilePath("/dev/null");
    addDockWidget(Qt::BottomDockWidgetArea, dockRBGOriginalWidget);
    dockRBGOriginalWidget->setFloating(true);
#ifdef INPAPER
    dockRBGOriginalWidget->setGeometry(830, 450,200,200);
#else
    int myx = 10;
    dockRBGOriginalWidget->setGeometry(830 - myx, 450 - 2 * myx, 350 + 2 * myx, 350 + 2 * myx);
#endif
#ifndef USER_STUDY
    dockRBGOriginalWidget->hide();
#endif

    dockRBGCurrentWidget = new QDockWidget();
    RGBWidget *rgbCurrentWidget = new RGBWidget(false);
    rgbCurrentWidget->setMinimumSize(100, 100);
    rgbCurrentWidget->setData(data);

    dockRBGCurrentWidget->setWidget(rgbCurrentWidget);
    dockRBGCurrentWidget->setWindowTitle("Recolored RGB");
    dockRBGCurrentWidget->setWindowIcon(QIcon(QPixmap(":/icons/ice--pencil.png")));
    dockRBGCurrentWidget->setWindowFilePath("/dev/null");
    addDockWidget(Qt::BottomDockWidgetArea, dockRBGCurrentWidget);
    dockRBGCurrentWidget->setFloating(true);
#ifdef INPAPER
    dockRBGCurrentWidget->setGeometry(1400, 450, 200, 200);
#else
    dockRBGCurrentWidget->setGeometry(1400 - myx, 450 - 2 * myx, 350 + 2 * myx, 350 + 2 * myx);
#endif
#ifndef USER_STUDY
    dockRBGCurrentWidget->hide();
#endif



	//RGB view===================================================================================================

	//Palette + time view========================================================================================

    PaletteViewWidget *paletteWidget = new PaletteViewWidget();
#ifdef INPAPER
    paletteWidget->setMinimumSize(500, 210);
#else
    paletteWidget->setMinimumSize(500, 250);
#endif
    paletteWidget->setData(data);

#ifndef USER_STUDY
    QDockWidget *dockPaletteWidget = new QDockWidget();

    dockPaletteWidget->setWidget(paletteWidget);
    dockPaletteWidget->setWindowFilePath("/dev/null");
    dockPaletteWidget->setWindowIcon(QIcon(QPixmap(":/icons/eraser.png")));
    dockPaletteWidget->setWindowTitle("Palette Timeline");
    dockPaletteWidget->setStyle(new IconnedDockStyle(QIcon(":/icons/eraser.png"), dockPaletteWidget->style()));


    addDockWidget(Qt::RightDockWidgetArea, dockPaletteWidget);
    dockPaletteWidget->setFloating(true);
    dockPaletteWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
#else
    {
        QHBoxLayout *LabelPaletteTimeLine = new QHBoxLayout;
        QLabel *label = new QLabel("Palette Timeline");
        QLabel *icon = new QLabel;
        icon->setPixmap(QPixmap(":/icons/film-timeline.png").scaled(16, 16, Qt::KeepAspectRatio));
#ifdef INPAPER
        QFont f = label->font();
        f.setBold(true);
        f.setPointSize(12);
        label->setFont(f);
#endif

        LabelPaletteTimeLine->addStretch();
        LabelPaletteTimeLine->addWidget(icon);
        LabelPaletteTimeLine->addWidget(label);
        LabelPaletteTimeLine->addStretch();

        mainLayout->addLayout(LabelPaletteTimeLine);

        // QHBoxLayout *layout = new QHBoxLayout;
        // QGroupBox *groupPaletteTimeLine = new QGroupBox(tr(""));
        // layout->addWidget(paletteWidget);
        // groupPaletteTimeLine->setLayout(layout);
        // mainLayout->addWidget(groupPaletteTimeLine);

        mainLayout->addWidget(paletteWidget);
    }
#endif

	slider->setMinimum(0);
	slider->setMaximum(0);
	//Palette + time view========================================================================================


	//Color picker===============================================================================================
	color_widgets::ColorDialog* colorDialog = new color_widgets::ColorDialog();
    colorDialog->setWindowTitle("Color Picker");

#ifndef USER_STUDY
	QDockWidget *dockColorWidget = new QDockWidget();
	dockColorWidget->setWidget(colorDialog);
    dockColorWidget->setWindowTitle("Color Picker");
	addDockWidget(Qt::RightDockWidgetArea, dockColorWidget);
	dockColorWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
#else
    {
        QHBoxLayout *LabelColorPicker = new QHBoxLayout;
        QLabel *label = new QLabel("Color Picker");
        QLabel *icon = new QLabel;
        icon->setPixmap(QPixmap(":/icons/color.png").scaled(16, 16, Qt::KeepAspectRatio));
#ifdef INPAPER
        QFont f = label->font();
        f.setBold(true);
        f.setPointSize(12);
        label->setFont(f);
#endif
        LabelColorPicker->addStretch();
        LabelColorPicker->addWidget(icon);
        LabelColorPicker->addWidget(label);
        LabelColorPicker->addStretch();

        mainLayout->addLayout(LabelColorPicker);

        QHBoxLayout *layout = new QHBoxLayout;
        QGroupBox *groupColorDialog = new QGroupBox();
        layout->addWidget(colorDialog);
        groupColorDialog->setLayout(layout);
        mainLayout->addWidget(groupColorDialog);
        // mainLayout->addWidget(colorDialog);
    }
#endif
	//Color picker===============================================================================================

#ifndef USER_STUDY
    mainLayout->addWidget(groupBox);
	mainLayout->addWidget(dockPaletteWidget);
    mainLayout->addWidget(dockColorWidget);
#else
#endif

	mainWidget->setLayout(mainLayout);
	this->setCentralWidget(mainWidget);

	//funcitons==================================================================================================
	connect(slider, &QSlider::valueChanged, original_video, &VideoWidget::setTime);
    connect(slider, &QSlider::valueChanged, recolored_video, &VideoWidget::setTime);
    connect(slider, &QSlider::valueChanged, rgbOriginalWidget, &RGBWidget::setTime);
    connect(slider, &QSlider::valueChanged, rgbCurrentWidget, &RGBWidget::setTime);
	connect(slider, &QSlider::valueChanged, paletteWidget, &PaletteViewWidget::setTime);
	connect(paletteWidget, &PaletteViewWidget::timeChanged, slider, &QSlider::setValue);

    connect(showVideoDataCheck, &QCheckBox::stateChanged, rgbOriginalWidget, &RGBWidget::setShowVideoData);
    connect(showVideoDataCheck, &QCheckBox::stateChanged, rgbCurrentWidget, &RGBWidget::setShowVideoData);
	connect(showVideoDataCheck, &QCheckBox::stateChanged, paletteWidget, &PaletteViewWidget::setShowVideoData);

    connect(videoPreviewSlider, &QSlider::valueChanged, rgbOriginalWidget, &RGBWidget::setPreview);
    connect(videoPreviewSlider, &QSlider::valueChanged, rgbCurrentWidget, &RGBWidget::setPreview);
    connect(videoPreviewSlider, &QSlider::valueChanged, paletteWidget, &PaletteViewWidget::setPreview);
    connect(videoPreviewSlider, &QSlider::valueChanged, [=](int x){this->preview = x / 1000.f;});

	connect(buttonPaleeteTimeBeforeOnly, &QRadioButton::clicked, [=]() {paletteWidget->setShowPalette(true, false); });
	connect(buttonPaleeteTimeAfterOnly, &QRadioButton::clicked, [=]() {paletteWidget->setShowPalette(false, true); });
	connect(buttonPaleeteTimeBoth, &QRadioButton::clicked, [=]() {paletteWidget->setShowPalette(true, true); });

	//show rgb view 
    connect(rgbButton, &QPushButton::clicked, [=]() {dockRBGOriginalWidget->show(); dockRBGCurrentWidget->show();});

	//set and get color from color dialog
	connect(colorDialog, &ColorDialog::colorChanged, paletteWidget, &PaletteViewWidget::getColor);
	connect(paletteWidget, &PaletteViewWidget::setColor, colorDialog, &ColorDialog::setColor);

    connect(colorDialog, &ColorDialog::colorChanged, rgbOriginalWidget, &RGBWidget::update);
    connect(colorDialog, &ColorDialog::colorChanged, rgbCurrentWidget, &RGBWidget::update);

	connect(openVideoAndPaletteButton, &QPushButton::clicked, [=]() { this->openFile(true); });
#ifdef USER_STUDY
    connect(openAction, &QAction::triggered, [=]() { this->openUserStudy(); });
    connect(saveAction, &QAction::triggered, [=]() { this->exportPalette(); });
    connect(exportAction, &QAction::triggered, [=]() { this->exportVideo(); });
    connect(resetAction, &QAction::triggered, paletteWidget, &PaletteViewWidget::resetVertex);
    connect(resetAction, &QAction::triggered, paletteWidget, &PaletteViewWidget::resetVertex);
    connect(discardAction, &QAction::triggered, paletteWidget, &PaletteViewWidget::resetAllVertex);
#endif
	connect(calWeightsBtn, &QPushButton::clicked, [=]() { this->calculateWeights(mvc_radio->isChecked(), rgbxy_radio->isChecked(), lbc_radio->isChecked()); });
	connect(importWeightsButton, &QPushButton::clicked, [=]() { this->importWeights(); });
	connect(saveWeightsBtn, &QPushButton::clicked, [=]() { this->exportWeights(); });
	connect(exportVideoeBtn, &QPushButton::clicked, [=]() { this->exportVideo(); });
    connect(exportTikZBtn, &QPushButton::clicked, [=]() { this->exportTikZ(); });

	connect(importPaletteButton, &QPushButton::clicked, [=]() { this->importPalette(); });
	connect(savePaletteBtn, &QPushButton::clicked, [=]() { this->exportPalette(); });

    setAcceptDrops(true);

    this->installEventFilter(paletteWidget);
    this->installEventFilter(this);
}

MainWindow::~MainWindow()
{
}

bool MainWindow::eventFilter(QObject *target, QEvent *e)
{
    Q_UNUSED(target);

    bool handled = false;
    if(e->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = (QKeyEvent *)e;

        qDebug() << "MainWindow::eventFilter" << keyEvent->key() << Qt::Key_Space;

        switch(keyEvent->key())
        {
        case Qt::Key_Space:
            // heightAdsorb = 1;
            break;
        }
    }
    return handled;
}

// open video & poly file
// TODO: replace input video file with H264 encoded video
void MainWindow::openFile(bool merge)
{
	if (data == nullptr) return;

	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::ExistingFile);
	dialog.setNameFilter(tr("*.video"));
	dialog.setViewMode(QFileDialog::Detail);

	if (dialog.exec())
	{
		QStringList fileName = dialog.selectedFiles();

		for (auto s : fileName)
			data->openVideo(QString(s));

		videoBeforeDockWidget->show();
		videoAfterDockWidget->show();
        // dockRBGOriginalWidget->show();
        // dockRBGCurrentWidget->show();
	}
	else
		return;

	dialog.setNameFilter("*.poly *.blockpoly *.polyface");

	// if shows merge step, open several poly files instead of one
	dialog.setFileMode(merge ? QFileDialog::ExistingFiles : QFileDialog::ExistingFile);

	if (dialog.exec())
	{
		QStringList fileName = dialog.selectedFiles();

		fileName.sort();
		data->resize();
		if (mergeStepSlider != nullptr)
			mergeStepSlider->setMaximum(fileName.size() - 1);

		int i = 0;
		for (auto s : fileName)
		{
			bool isPolyface = s.endsWith("polyface");
			data->openPoly(QString(s), isPolyface);
			i++;
		}
		data->recalcPaletteHeight();
	}
	else
		return;

	if (slider != nullptr)
		slider->setMaximum(data->getFrmCnt() - 1);
}


void MainWindow::importWeights() {
	if (data == nullptr) return;

	QFileDialog fileDialog(this);
	fileDialog.setFileMode(QFileDialog::DirectoryOnly);
	QStringList dirName;
	if (fileDialog.exec() == QDialog::Accepted)
	{
		dirName = fileDialog.selectedFiles();
		string dirNamestr = dirName[0].toStdString();
		data->ImportWeights(dirNamestr);
	}
}

void MainWindow::exportWeights() {
	if (data == nullptr) return;

	QFileDialog fileDialog(this);
	fileDialog.setFileMode(QFileDialog::DirectoryOnly);
	QStringList dirName;
	if (fileDialog.exec() == QDialog::Accepted)
	{
		dirName = fileDialog.selectedFiles();
		string dirNamestr = dirName[0].toStdString();
		data->ExportWeights(dirNamestr);
		int x = 0;
	}
}

void MainWindow::importPalette() {
	if (data == nullptr) return;

	QFileDialog fileDialog(this);
	fileDialog.setFileMode(QFileDialog::QFileDialog::ExistingFile);
    fileDialog.setNameFilter(tr("*.txt;*.palettev2;*.palettev3;*.palettev4"));
	fileDialog.setViewMode(QFileDialog::Detail);

	QStringList dirName;
	if (fileDialog.exec() == QDialog::Accepted)
	{
		dirName = fileDialog.selectedFiles();
		string dirNamestr = dirName[0].toStdString();
        data->ImportChangedPalette(dirNamestr);
	}
}

void MainWindow::exportPalette() {
	if (data == nullptr) return;

    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setNameFilter(tr("*.palettev4"));
    fileDialog.setViewMode(QFileDialog::Detail);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.selectFile("save");
    if (fileDialog.exec() == QDialog::Accepted)
    {
        for(auto fileName: fileDialog.selectedFiles())
        {
            data->ExportChangedPalette(fileName.toStdString());
        }
        QMessageBox::information(NULL, "Info", "Successfully Saved Palette", QMessageBox::Yes, QMessageBox::Yes);
    }
}

void MainWindow::calculateWeights(bool mvc_, bool rgbxy_, bool lbc_) {
	if (mvc_)
        data->ComputeMVCWeights();
	else if (rgbxy_)
		data->ComputeRgbxyOrLbcWeights(RGBXY);
		//data->ComputeRgbxyWeights();
	else 
		data->ComputeRgbxyOrLbcWeights(LBC);
}

void MainWindow::exportVideo() {
	if (data == nullptr) return;

	QFileDialog fileDialog(this);
	fileDialog.setFileMode(QFileDialog::DirectoryOnly);
	QStringList dirName;
	if (fileDialog.exec() == QDialog::Accepted)
	{
		dirName = fileDialog.selectedFiles();
		string dirNamestr = dirName[0].toStdString();

        string recolor_dir = dirNamestr + "/recolored";
        QDir(QString(dirNamestr.c_str())).mkdir("recolored");
		data->ExportRecoloredVideo(recolor_dir);
		system(("python scripts/image2mp4.py " + recolor_dir + " " + recolor_dir).c_str());

		string original_dir = dirNamestr + "/original";
        QDir(QString(dirNamestr.c_str())).mkdir("original");
		data->ExportOriginalVideo(original_dir);
		system(("python scripts/image2mp4.py " + original_dir + " " + original_dir).c_str());

        QMessageBox::information(NULL, "Info", "Export Video finished", QMessageBox::Yes, QMessageBox::Yes);
	}
}

void MainWindow::exportTikZ()
{
    if (data == nullptr) return;
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::DirectoryOnly);

    if (fileDialog.exec() == QDialog::Accepted)
    {
        QStringList dirNames = fileDialog.selectedFiles();

        for(QString dirName : dirNames)
        {
            data->ExportTikz(dirName, preview);
        }
    }

}

void MainWindow::openUserStudy()
{
    if (data == nullptr) return;

    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::DirectoryOnly);
    QStringList dirName;
    if (fileDialog.exec() == QDialog::Accepted)
    {
        dirName = fileDialog.selectedFiles();
        string dirNamestr = dirName.rbegin()->toStdString();

        data->openVideo(QString((dirNamestr + "/frames.video").c_str()));

        data->resize();
        data->openPoly(QString((dirNamestr + "/polyhedron.polyface").c_str()), true);
        data->recalcPaletteHeight();
        if (slider != nullptr)
            slider->setMaximum(data->getFrmCnt() - 1);

        data->ImportChangedPalette(dirNamestr + "/init.palettev3");
        data->ImportWeights(dirNamestr + "/weight/");
    }

    // this->openFile(true);
    // this->importPalette();
    // this->importWeights();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    // qDebug() << "dragEnterEvent = " << event->mimeData()->urls();
    event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *e)
{
    qDebug() << "dropEvent";

    foreach (const QUrl &url, e->mimeData()->urls()) {
        QString fileName = url.toLocalFile();
        // qDebug() << "drop fileName = " << fileName;
        if(fileName.endsWith(".video"))
        {
            data->openVideo(fileName);

            videoBeforeDockWidget->show();
            videoAfterDockWidget->show();
        }
    }

    foreach (const QUrl &url, e->mimeData()->urls()) {
        QString fileName = url.toLocalFile();
        if(fileName.endsWith(".poly"))
        {
            data->resize();
            data->openPoly(fileName, false);
            data->recalcPaletteHeight();
            if (slider != nullptr)
                slider->setMaximum(data->getFrmCnt() - 1);
        }
        else if(fileName.endsWith(".polyface"))
        {
            data->resize();
            data->openPoly(fileName, true);
            data->recalcPaletteHeight();
            if (slider != nullptr)
                slider->setMaximum(data->getFrmCnt() - 1);
        }
    }

    foreach (const QUrl &url, e->mimeData()->urls()) {
        QString fileName = url.toLocalFile();
        if(fileName.endsWith(".palettev2") || fileName.endsWith(".palettev3") ||
                fileName.endsWith(".palettev4"))
        {
            data->ImportChangedPalette(fileName.toStdString());
        }

        qDebug() << fileName;
    }

    foreach (const QUrl &url, e->mimeData()->urls()) {
        QString fileName = url.toLocalFile();
        if(fileName.endsWith("weight/"))
        {
            data->ImportWeights(fileName.toStdString());
        }

        qDebug() << fileName;
    }

    e->acceptProposedAction();
}
