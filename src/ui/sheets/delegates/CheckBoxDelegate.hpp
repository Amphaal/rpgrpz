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