#pragma once

#include "src/ui/communication/chat/logs/UsersLog.h"

#include <QPainter>

#include <QStyledItemDelegate>

#include "src/helpers/RPZUserRoles.h"

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
    
    private:
        static inline QPixmap* _hiddenPix = nullptr;
        static inline QPixmap* _lockPix = nullptr;
};