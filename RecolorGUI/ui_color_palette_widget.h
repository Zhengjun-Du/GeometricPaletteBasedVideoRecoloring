/********************************************************************************
** Form generated from reading UI file 'color_palette_widget.ui'
**
** Created by: Qt User Interface Compiler version 5.14.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_COLOR_PALETTE_WIDGET_H
#define UI_COLOR_PALETTE_WIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "QtColorWidgets/swatch.hpp"

namespace color_widgets {

class Ui_ColorPaletteWidget
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_3;
    QComboBox *palette_list;
    QWidget *group_edit_list;
    QHBoxLayout *horizontalLayout_2;
    QToolButton *button_palette_open;
    QToolButton *button_palette_new;
    QToolButton *button_palette_duplicate;
    color_widgets::Swatch *swatch;
    QWidget *group_edit_palette;
    QHBoxLayout *horizontalLayout;
    QToolButton *button_palette_delete;
    QToolButton *button_palette_revert;
    QToolButton *button_palette_save;
    QSpacerItem *horizontalSpacer;
    QToolButton *button_color_add;
    QToolButton *button_color_remove;

    void setupUi(QWidget *color_widgets__ColorPaletteWidget)
    {
        if (color_widgets__ColorPaletteWidget->objectName().isEmpty())
            color_widgets__ColorPaletteWidget->setObjectName(QString::fromUtf8("color_widgets__ColorPaletteWidget"));
        color_widgets__ColorPaletteWidget->resize(227, 186);
        verticalLayout = new QVBoxLayout(color_widgets__ColorPaletteWidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        palette_list = new QComboBox(color_widgets__ColorPaletteWidget);
        palette_list->setObjectName(QString::fromUtf8("palette_list"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(palette_list->sizePolicy().hasHeightForWidth());
        palette_list->setSizePolicy(sizePolicy);

        horizontalLayout_3->addWidget(palette_list);

        group_edit_list = new QWidget(color_widgets__ColorPaletteWidget);
        group_edit_list->setObjectName(QString::fromUtf8("group_edit_list"));
        horizontalLayout_2 = new QHBoxLayout(group_edit_list);
        horizontalLayout_2->setSpacing(0);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        button_palette_open = new QToolButton(group_edit_list);
        button_palette_open->setObjectName(QString::fromUtf8("button_palette_open"));
        QIcon icon;
        QString iconThemeName = QString::fromUtf8("document-open");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon = QIcon::fromTheme(iconThemeName);
        } else {
            icon.addFile(QString::fromUtf8(""), QSize(), QIcon::Normal, QIcon::Off);
        }
        button_palette_open->setIcon(icon);

        horizontalLayout_2->addWidget(button_palette_open);

        button_palette_new = new QToolButton(group_edit_list);
        button_palette_new->setObjectName(QString::fromUtf8("button_palette_new"));
        QIcon icon1;
        iconThemeName = QString::fromUtf8("document-new");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon1 = QIcon::fromTheme(iconThemeName);
        } else {
            icon1.addFile(QString::fromUtf8(""), QSize(), QIcon::Normal, QIcon::Off);
        }
        button_palette_new->setIcon(icon1);

        horizontalLayout_2->addWidget(button_palette_new);

        button_palette_duplicate = new QToolButton(group_edit_list);
        button_palette_duplicate->setObjectName(QString::fromUtf8("button_palette_duplicate"));
        QIcon icon2;
        iconThemeName = QString::fromUtf8("edit-copy");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon2 = QIcon::fromTheme(iconThemeName);
        } else {
            icon2.addFile(QString::fromUtf8(""), QSize(), QIcon::Normal, QIcon::Off);
        }
        button_palette_duplicate->setIcon(icon2);

        horizontalLayout_2->addWidget(button_palette_duplicate);


        horizontalLayout_3->addWidget(group_edit_list);


        verticalLayout->addLayout(horizontalLayout_3);

        swatch = new color_widgets::Swatch(color_widgets__ColorPaletteWidget);
        swatch->setObjectName(QString::fromUtf8("swatch"));

        verticalLayout->addWidget(swatch);

        group_edit_palette = new QWidget(color_widgets__ColorPaletteWidget);
        group_edit_palette->setObjectName(QString::fromUtf8("group_edit_palette"));
        horizontalLayout = new QHBoxLayout(group_edit_palette);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        button_palette_delete = new QToolButton(group_edit_palette);
        button_palette_delete->setObjectName(QString::fromUtf8("button_palette_delete"));
        QIcon icon3;
        iconThemeName = QString::fromUtf8("document-close");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon3 = QIcon::fromTheme(iconThemeName);
        } else {
            icon3.addFile(QString::fromUtf8(""), QSize(), QIcon::Normal, QIcon::Off);
        }
        button_palette_delete->setIcon(icon3);

        horizontalLayout->addWidget(button_palette_delete);

        button_palette_revert = new QToolButton(group_edit_palette);
        button_palette_revert->setObjectName(QString::fromUtf8("button_palette_revert"));
        QIcon icon4;
        iconThemeName = QString::fromUtf8("document-revert");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon4 = QIcon::fromTheme(iconThemeName);
        } else {
            icon4.addFile(QString::fromUtf8(""), QSize(), QIcon::Normal, QIcon::Off);
        }
        button_palette_revert->setIcon(icon4);

        horizontalLayout->addWidget(button_palette_revert);

        button_palette_save = new QToolButton(group_edit_palette);
        button_palette_save->setObjectName(QString::fromUtf8("button_palette_save"));
        QIcon icon5;
        iconThemeName = QString::fromUtf8("document-save");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon5 = QIcon::fromTheme(iconThemeName);
        } else {
            icon5.addFile(QString::fromUtf8(""), QSize(), QIcon::Normal, QIcon::Off);
        }
        button_palette_save->setIcon(icon5);

        horizontalLayout->addWidget(button_palette_save);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        button_color_add = new QToolButton(group_edit_palette);
        button_color_add->setObjectName(QString::fromUtf8("button_color_add"));
        QIcon icon6;
        iconThemeName = QString::fromUtf8("list-add");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon6 = QIcon::fromTheme(iconThemeName);
        } else {
            icon6.addFile(QString::fromUtf8(""), QSize(), QIcon::Normal, QIcon::Off);
        }
        button_color_add->setIcon(icon6);

        horizontalLayout->addWidget(button_color_add);

        button_color_remove = new QToolButton(group_edit_palette);
        button_color_remove->setObjectName(QString::fromUtf8("button_color_remove"));
        QIcon icon7;
        iconThemeName = QString::fromUtf8("list-remove");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon7 = QIcon::fromTheme(iconThemeName);
        } else {
            icon7.addFile(QString::fromUtf8(""), QSize(), QIcon::Normal, QIcon::Off);
        }
        button_color_remove->setIcon(icon7);

        horizontalLayout->addWidget(button_color_remove);


        verticalLayout->addWidget(group_edit_palette);


        retranslateUi(color_widgets__ColorPaletteWidget);

        QMetaObject::connectSlotsByName(color_widgets__ColorPaletteWidget);
    } // setupUi

    void retranslateUi(QWidget *color_widgets__ColorPaletteWidget)
    {
#if QT_CONFIG(tooltip)
        button_palette_open->setToolTip(QCoreApplication::translate("color_widgets::ColorPaletteWidget", "Open a new palette from file", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        button_palette_new->setToolTip(QCoreApplication::translate("color_widgets::ColorPaletteWidget", "Create a new palette", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        button_palette_duplicate->setToolTip(QCoreApplication::translate("color_widgets::ColorPaletteWidget", "Duplicate the current palette", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        button_palette_delete->setToolTip(QCoreApplication::translate("color_widgets::ColorPaletteWidget", "Delete the current palette", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        button_palette_revert->setToolTip(QCoreApplication::translate("color_widgets::ColorPaletteWidget", "Revert changes to the current palette", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        button_palette_save->setToolTip(QCoreApplication::translate("color_widgets::ColorPaletteWidget", "Save changes to the current palette", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        button_color_add->setToolTip(QCoreApplication::translate("color_widgets::ColorPaletteWidget", "Add a color to the palette", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        button_color_remove->setToolTip(QCoreApplication::translate("color_widgets::ColorPaletteWidget", "Remove the selected color from the palette", nullptr));
#endif // QT_CONFIG(tooltip)
        (void)color_widgets__ColorPaletteWidget;
    } // retranslateUi

};

} // namespace color_widgets

namespace color_widgets {
namespace Ui {
    class ColorPaletteWidget: public Ui_ColorPaletteWidget {};
} // namespace Ui
} // namespace color_widgets

#endif // UI_COLOR_PALETTE_WIDGET_H
