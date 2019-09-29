#pragma once

#include <QStyledItemDelegate>
#include <QSpinBox>

class SpinBoxDelegate : public QStyledItemDelegate {

    public: 
        SpinBoxDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) { }

        QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
            QSpinBox *editor = new QSpinBox(parent);
            editor->setFrame(false);
            return editor;
        }

        void setEditorData(QWidget *editor, const QModelIndex &index) const override {
            int value = index.model()->data(index, Qt::EditRole).toInt();

            QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
            spinBox->setValue(value);
        }

        void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
            QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
            spinBox->interpretText();
            int value = spinBox->value();

            model->setData(index, value, Qt::EditRole);
        }

        void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
            editor->setGeometry(option.rect);
        }
};