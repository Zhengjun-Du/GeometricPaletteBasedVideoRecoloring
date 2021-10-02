/**
 * \file
 *
 * \author Mattia Basaglia
 *
 * \copyright Copyright (C) 2013-2020 Mattia Basaglia
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "QtColorWidgets/color_delegate.hpp"
#include "QtColorWidgets/color_selector.hpp"
#include "QtColorWidgets/color_dialog.hpp"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>

namespace color_widgets {

ColorDelegate::ColorDelegate(QWidget *parent) :
    QStyledItemDelegate(parent)
{
}

void ColorDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
{
    if ( index.data().type() == QVariant::Color )
    {
        QStyleOptionFrame panel;
        panel.initFrom(option.widget);
        if (option.widget->isEnabled())
            panel.state = QStyle::State_Enabled;
        panel.rect = option.rect;
        panel.lineWidth = 2;
        panel.midLineWidth = 0;
        panel.state |= QStyle::State_Sunken;
        option.widget->style()->drawPrimitive(QStyle::PE_Frame, &panel, painter, nullptr);
        QRect r = option.widget->style()->subElementRect(QStyle::SE_FrameContents, &panel, nullptr);
        painter->fillRect(r, index.data().value<QColor>());
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize ColorDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if ( index.data().type() == QVariant::Color )
        return QSize(24,16);
    return QStyledItemDelegate::sizeHint(option, index);
}

QWidget *ColorDelegate::createEditor(QWidget *parent,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    if ( index.data().type() == QVariant::Color )
    {
        ColorDialog *editor = new ColorDialog(const_cast<QWidget*>(parent));
        connect(editor, &QDialog::accepted, this, &ColorDelegate::close_editor);
        connect(editor, &ColorDialog::colorSelected, this, &ColorDelegate::color_changed);
        return editor;
    }

    return QStyledItemDelegate::createEditor(parent, option, index);
}

void ColorDelegate::color_changed()
{
    ColorDialog *editor = qobject_cast<ColorDialog*>(sender());
    emit commitData(editor);
}

void ColorDelegate::close_editor()
{
    ColorDialog *editor = qobject_cast<ColorDialog*>(sender());
    emit closeEditor(editor);
}

void ColorDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{

    if ( index.data().type() == QVariant::Color )
    {
        ColorDialog *selector = qobject_cast<ColorDialog*>(editor);
        selector->setColor(qvariant_cast<QColor>(index.data()));
        return;
    }

    QStyledItemDelegate::setEditorData(editor, index);
}

void ColorDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                  const QModelIndex &index) const
{
    if ( index.data().type() == QVariant::Color )
    {
        ColorDialog *selector = qobject_cast<ColorDialog *>(editor);
        model->setData(index, QVariant::fromValue(selector->color()));
        return;
    }

    QStyledItemDelegate::setModelData(editor, model, index);
}

void ColorDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    if ( index.data().type() == QVariant::Color )
    {
        return;
    }


    QStyledItemDelegate::updateEditorGeometry(editor, option, index);

}

} // namespace color_widgets
