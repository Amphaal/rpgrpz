#pragma once

#include "src/ui/communication/logs/UsersLog.hpp"

#include <QPainter>

#include <QStyledItemDelegate>

class OwnerDelegate  : public QStyledItemDelegate {
    public:
        OwnerDelegate(QWidget *parent = nullptr) : QStyledItemDelegate(parent) {}

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
            
            auto color = index.data(Qt::UserRole).value<QColor>();

            if(color.isValid()) {
                
                //define colors
                painter->setBrush(QBrush(color, Qt::SolidPattern));

                //define size
                QRect indicator(
                    QPoint(option.rect.left()+4, option.rect.top()+4),
                    QPoint(option.rect.right()-4, option.rect.bottom()-4)
                );

                //draw
                return painter->drawRect(indicator);
            }

            QStyledItemDelegate::paint(painter, option, index);
        }

        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
            return QStyledItemDelegate::sizeHint(option, index);
        }
};

class LockAndVisibilityDelegate  : public QStyledItemDelegate {
    public:
        LockAndVisibilityDelegate(QWidget *parent = nullptr) : QStyledItemDelegate(parent) { }

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
            
            //fetch data
            auto data = index.data(Qt::UserRole).toList();
            auto isHidden = data[0].toBool();
            auto isLocked = data[1].toBool();

            if(isHidden) painter->drawPixmap(option.rect.topLeft(), QPixmap(":/icons/app/tools/hidden.png"));
            if(isLocked) painter->drawPixmap(option.rect.topRight(), QPixmap(":/icons/app/tools/lock.png"));

            //TODO add interactions
            // return QStyledItemDelegate::paint(painter, option, index);
        }

        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
            return QStyledItemDelegate::sizeHint(option, index);
        }
};