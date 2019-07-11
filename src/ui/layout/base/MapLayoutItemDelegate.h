#pragma once

#include "src/ui/communication/chat/logs/UsersLog.h"

#include <QPainter>

#include <QStyledItemDelegate>

enum LayoutCustomRoles {
    OwnerIdRole = 267, 
    RPZAssetIdRole = 260, 
    VisibilityRole = 261, 
    AvailabilityRole = 262 
};

class OwnerDelegate  : public QStyledItemDelegate {
    
    public:
        OwnerDelegate(QWidget *parent = nullptr);

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class LockAndVisibilityDelegate  : public QStyledItemDelegate {
    
    public:
        LockAndVisibilityDelegate(QWidget *parent = nullptr);

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};