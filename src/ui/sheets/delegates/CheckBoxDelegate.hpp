// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

#pragma once

#include <QStyledItemDelegate>
#include <QCheckBox>

class CheckBoxDelegate : public QStyledItemDelegate {

 public: 
        CheckBoxDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) { }

        QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
            QCheckBox *editor = new QCheckBox(parent);
            return editor;
        }

        void setEditorData(QWidget *editor, const QModelIndex &index) const override {
            auto value = index.model()->data(index, Qt::EditRole).toBool();
            QCheckBox *spinBox = static_cast<QCheckBox*>(editor);
            spinBox->setChecked(value);
        }

        void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
            QCheckBox *spinBox = static_cast<QCheckBox*>(editor);
            auto value = spinBox->isChecked();
            model->setData(index, value, Qt::EditRole);
        }

        void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override {           
            //center editor
            auto c = option.rect;
            c.moveLeft(c.center().x() - editor->sizeHint().width() / 2);           
            editor->setGeometry(c);
        }

        QString displayText(const QVariant &value, const QLocale &locale) const override {
            auto val = value.toBool();
            return val ? QString::fromUtf8("☑") : QString::fromUtf8("☐");
        }

};