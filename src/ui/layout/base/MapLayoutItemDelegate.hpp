#pragma once

#include "src/ui/communication/logs/UsersLog.hpp"

#include <QPainter>

#include <QStyledItemDelegate>

enum LayoutCustomRoles { 
    AssetIdRole = 260, 
    VisibilityRole = 261, 
    AvailabilityRole = 262 
};

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
            auto isHidden = index.data(VisibilityRole).toBool();
            auto isLocked = index.data(AvailabilityRole).toBool();

            if(isHidden) painter->drawPixmap(option.rect.topLeft(), QPixmap(":/icons/app/tools/hidden.png"));
            if(isLocked) painter->drawPixmap(QPoint(option.rect.right()-16, option.rect.top()), QPixmap(":/icons/app/tools/lock.png"));

        }

        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
            return QSize(32, 16);
        }
};