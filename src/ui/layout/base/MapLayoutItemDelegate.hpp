#pragma once

#include "src/ui/communication/logs/UsersLog.hpp"

#include <QPainter>

#include <QStyledItemDelegate>

class MapLayoutItemDelegate  : public QStyledItemDelegate {
    public:
        MapLayoutItemDelegate(QWidget *parent = nullptr) : QStyledItemDelegate(parent) {}

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
                painter->drawRect(indicator);
                qDebug() << "drawn";
            }

            QStyledItemDelegate::paint(painter, option, index);
        }

        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
            return QStyledItemDelegate::sizeHint(option, index);
        }
};