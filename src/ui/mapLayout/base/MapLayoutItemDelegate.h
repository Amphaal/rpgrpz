#pragma once

#include <QPainter>

#include <QStyledItemDelegate>

#include "src/helpers/RPZUserRoles.h"

class LockAndVisibilityDelegate  : public QStyledItemDelegate {
    
    public:
        LockAndVisibilityDelegate(QWidget *parent = nullptr);

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    
    private:
        static inline QPixmap* _hiddenPix = nullptr;
        static inline QPixmap* _lockPix = nullptr;
};