/********************************************************************************
** Form generated from reading UI file 'color_dialog.ui'
**
** Created by: Qt User Interface Compiler version 5.14.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_COLOR_DIALOG_H
#define UI_COLOR_DIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include "QtColorWidgets/color_line_edit.hpp"
#include "QtColorWidgets/color_preview.hpp"
#include "QtColorWidgets/color_wheel.hpp"
#include "QtColorWidgets/gradient_slider.hpp"
#include "QtColorWidgets/hue_slider.hpp"

QT_BEGIN_NAMESPACE

class Ui_ColorDialog
{
public:
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout;
    color_widgets::ColorWheel *wheel;
    color_widgets::ColorPreview *preview;
    QGridLayout *gridLayout;
    color_widgets::GradientSlider *slide_value;
    QSpinBox *spin_hue;
    QLabel *label_5;
    QLabel *label_6;
    color_widgets::HueSlider *slide_hue;
    QLabel *label;
    color_widgets::GradientSlider *slide_red;
    QFrame *line_alpha;
    QFrame *line;
    QSpinBox *spin_red;
    color_widgets::GradientSlider *slide_green;
    QLabel *label_7;
    QSpinBox *spin_saturation;
    color_widgets::GradientSlider *slide_alpha;
    color_widgets::ColorLineEdit *edit_hex;
    QLabel *label_8;
    color_widgets::GradientSlider *slide_saturation;
    QLabel *label_alpha;
    QSpinBox *spin_alpha;
    QSpinBox *spin_blue;
    QSpinBox *spin_value;
    QFrame *line_3;
    QLabel *label_2;
    color_widgets::GradientSlider *slide_blue;
    QSpinBox *spin_green;
    QLabel *label_3;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *ColorDialog)
    {
        if (ColorDialog->objectName().isEmpty())
            ColorDialog->setObjectName(QString::fromUtf8("ColorDialog"));
        ColorDialog->resize(491, 269);
        QIcon icon;
        QString iconThemeName = QString::fromUtf8("format-fill-color");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon = QIcon::fromTheme(iconThemeName);
        } else {
            icon.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        ColorDialog->setWindowIcon(icon);
        verticalLayout_2 = new QVBoxLayout(ColorDialog);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        wheel = new color_widgets::ColorWheel(ColorDialog);
        wheel->setObjectName(QString::fromUtf8("wheel"));
        QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(wheel->sizePolicy().hasHeightForWidth());
        wheel->setSizePolicy(sizePolicy);

        verticalLayout->addWidget(wheel);

        preview = new color_widgets::ColorPreview(ColorDialog);
        preview->setObjectName(QString::fromUtf8("preview"));

        verticalLayout->addWidget(preview);


        horizontalLayout->addLayout(verticalLayout);

        gridLayout = new QGridLayout();
        gridLayout->setSpacing(6);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        slide_value = new color_widgets::GradientSlider(ColorDialog);
        slide_value->setObjectName(QString::fromUtf8("slide_value"));
        slide_value->setMaximum(255);
        slide_value->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(slide_value, 2, 1, 1, 1);

        spin_hue = new QSpinBox(ColorDialog);
        spin_hue->setObjectName(QString::fromUtf8("spin_hue"));
        spin_hue->setWrapping(true);
        spin_hue->setMaximum(359);

        gridLayout->addWidget(spin_hue, 0, 2, 1, 1);

        label_5 = new QLabel(ColorDialog);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout->addWidget(label_5, 2, 0, 1, 1);

        label_6 = new QLabel(ColorDialog);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        gridLayout->addWidget(label_6, 0, 0, 1, 1);

        slide_hue = new color_widgets::HueSlider(ColorDialog);
        slide_hue->setObjectName(QString::fromUtf8("slide_hue"));
        slide_hue->setMinimum(0);
        slide_hue->setMaximum(359);

        gridLayout->addWidget(slide_hue, 0, 1, 1, 1);

        label = new QLabel(ColorDialog);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 4, 0, 1, 1);

        slide_red = new color_widgets::GradientSlider(ColorDialog);
        slide_red->setObjectName(QString::fromUtf8("slide_red"));
        slide_red->setMaximum(255);
        slide_red->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(slide_red, 4, 1, 1, 1);

        line_alpha = new QFrame(ColorDialog);
        line_alpha->setObjectName(QString::fromUtf8("line_alpha"));
        line_alpha->setFrameShape(QFrame::HLine);
        line_alpha->setFrameShadow(QFrame::Sunken);

        gridLayout->addWidget(line_alpha, 7, 0, 1, 3);

        line = new QFrame(ColorDialog);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        gridLayout->addWidget(line, 3, 0, 1, 3);

        spin_red = new QSpinBox(ColorDialog);
        spin_red->setObjectName(QString::fromUtf8("spin_red"));
        spin_red->setMaximum(255);

        gridLayout->addWidget(spin_red, 4, 2, 1, 1);

        slide_green = new color_widgets::GradientSlider(ColorDialog);
        slide_green->setObjectName(QString::fromUtf8("slide_green"));
        slide_green->setMaximum(255);
        slide_green->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(slide_green, 5, 1, 1, 1);

        label_7 = new QLabel(ColorDialog);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        gridLayout->addWidget(label_7, 1, 0, 1, 1);

        spin_saturation = new QSpinBox(ColorDialog);
        spin_saturation->setObjectName(QString::fromUtf8("spin_saturation"));
        spin_saturation->setMaximum(255);

        gridLayout->addWidget(spin_saturation, 1, 2, 1, 1);

        slide_alpha = new color_widgets::GradientSlider(ColorDialog);
        slide_alpha->setObjectName(QString::fromUtf8("slide_alpha"));
        slide_alpha->setMaximum(255);
        slide_alpha->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(slide_alpha, 8, 1, 1, 1);

        edit_hex = new color_widgets::ColorLineEdit(ColorDialog);
        edit_hex->setObjectName(QString::fromUtf8("edit_hex"));
        QFont font;
        font.setFamily(QString::fromUtf8("Monospace"));
        edit_hex->setFont(font);
        edit_hex->setProperty("showAlpha", QVariant(true));

        gridLayout->addWidget(edit_hex, 10, 1, 1, 2);

        label_8 = new QLabel(ColorDialog);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        gridLayout->addWidget(label_8, 10, 0, 1, 1);

        slide_saturation = new color_widgets::GradientSlider(ColorDialog);
        slide_saturation->setObjectName(QString::fromUtf8("slide_saturation"));
        slide_saturation->setMaximum(255);
        slide_saturation->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(slide_saturation, 1, 1, 1, 1);

        label_alpha = new QLabel(ColorDialog);
        label_alpha->setObjectName(QString::fromUtf8("label_alpha"));

        gridLayout->addWidget(label_alpha, 8, 0, 1, 1);

        spin_alpha = new QSpinBox(ColorDialog);
        spin_alpha->setObjectName(QString::fromUtf8("spin_alpha"));
        spin_alpha->setMaximum(255);

        gridLayout->addWidget(spin_alpha, 8, 2, 1, 1);

        spin_blue = new QSpinBox(ColorDialog);
        spin_blue->setObjectName(QString::fromUtf8("spin_blue"));
        spin_blue->setMaximum(255);

        gridLayout->addWidget(spin_blue, 6, 2, 1, 1);

        spin_value = new QSpinBox(ColorDialog);
        spin_value->setObjectName(QString::fromUtf8("spin_value"));
        spin_value->setMaximum(255);

        gridLayout->addWidget(spin_value, 2, 2, 1, 1);

        line_3 = new QFrame(ColorDialog);
        line_3->setObjectName(QString::fromUtf8("line_3"));
        line_3->setFrameShape(QFrame::HLine);
        line_3->setFrameShadow(QFrame::Sunken);

        gridLayout->addWidget(line_3, 9, 0, 1, 3);

        label_2 = new QLabel(ColorDialog);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 5, 0, 1, 1);

        slide_blue = new color_widgets::GradientSlider(ColorDialog);
        slide_blue->setObjectName(QString::fromUtf8("slide_blue"));
        slide_blue->setMaximum(255);
        slide_blue->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(slide_blue, 6, 1, 1, 1);

        spin_green = new QSpinBox(ColorDialog);
        spin_green->setObjectName(QString::fromUtf8("spin_green"));
        spin_green->setMaximum(255);

        gridLayout->addWidget(spin_green, 5, 2, 1, 1);

        label_3 = new QLabel(ColorDialog);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 6, 0, 1, 1);


        horizontalLayout->addLayout(gridLayout);


        verticalLayout_2->addLayout(horizontalLayout);

        buttonBox = new QDialogButtonBox(ColorDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setStandardButtons(QDialogButtonBox::NoButton);

        verticalLayout_2->addWidget(buttonBox);


        retranslateUi(ColorDialog);
        QObject::connect(slide_saturation, SIGNAL(valueChanged(int)), ColorDialog, SLOT(set_hsv()));
        QObject::connect(slide_value, SIGNAL(valueChanged(int)), ColorDialog, SLOT(set_hsv()));
        QObject::connect(slide_red, SIGNAL(valueChanged(int)), ColorDialog, SLOT(set_rgb()));
        QObject::connect(slide_green, SIGNAL(valueChanged(int)), ColorDialog, SLOT(set_rgb()));
        QObject::connect(slide_blue, SIGNAL(valueChanged(int)), ColorDialog, SLOT(set_rgb()));
        QObject::connect(slide_alpha, SIGNAL(valueChanged(int)), ColorDialog, SLOT(set_alpha()));
        QObject::connect(wheel, SIGNAL(colorSelected(QColor)), ColorDialog, SLOT(setColorInternal(QColor)));
        QObject::connect(slide_saturation, SIGNAL(valueChanged(int)), spin_saturation, SLOT(setValue(int)));
        QObject::connect(spin_saturation, SIGNAL(valueChanged(int)), slide_saturation, SLOT(setValue(int)));
        QObject::connect(slide_value, SIGNAL(valueChanged(int)), spin_value, SLOT(setValue(int)));
        QObject::connect(spin_value, SIGNAL(valueChanged(int)), slide_value, SLOT(setValue(int)));
        QObject::connect(slide_red, SIGNAL(valueChanged(int)), spin_red, SLOT(setValue(int)));
        QObject::connect(spin_red, SIGNAL(valueChanged(int)), slide_red, SLOT(setValue(int)));
        QObject::connect(slide_green, SIGNAL(valueChanged(int)), spin_green, SLOT(setValue(int)));
        QObject::connect(spin_green, SIGNAL(valueChanged(int)), slide_green, SLOT(setValue(int)));
        QObject::connect(slide_alpha, SIGNAL(valueChanged(int)), spin_alpha, SLOT(setValue(int)));
        QObject::connect(spin_alpha, SIGNAL(valueChanged(int)), slide_alpha, SLOT(setValue(int)));
        QObject::connect(slide_blue, SIGNAL(valueChanged(int)), spin_blue, SLOT(setValue(int)));
        QObject::connect(spin_blue, SIGNAL(valueChanged(int)), slide_blue, SLOT(setValue(int)));
        QObject::connect(slide_hue, SIGNAL(valueChanged(int)), spin_hue, SLOT(setValue(int)));
        QObject::connect(spin_hue, SIGNAL(valueChanged(int)), slide_hue, SLOT(setValue(int)));
        QObject::connect(slide_hue, SIGNAL(valueChanged(int)), ColorDialog, SLOT(set_hsv()));

        QMetaObject::connectSlotsByName(ColorDialog);
    } // setupUi

    void retranslateUi(QDialog *ColorDialog)
    {
        ColorDialog->setWindowTitle(QCoreApplication::translate("ColorDialog", "Select Color", nullptr));
        label_5->setText(QCoreApplication::translate("ColorDialog", "Value", nullptr));
        label_6->setText(QCoreApplication::translate("ColorDialog", "Hue", nullptr));
        label->setText(QCoreApplication::translate("ColorDialog", "Red", nullptr));
        label_7->setText(QCoreApplication::translate("ColorDialog", "Saturation", nullptr));
        label_8->setText(QCoreApplication::translate("ColorDialog", "Hex", nullptr));
        label_alpha->setText(QCoreApplication::translate("ColorDialog", "Alpha", nullptr));
        label_2->setText(QCoreApplication::translate("ColorDialog", "Green", nullptr));
        label_3->setText(QCoreApplication::translate("ColorDialog", "Blue", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ColorDialog: public Ui_ColorDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COLOR_DIALOG_H
