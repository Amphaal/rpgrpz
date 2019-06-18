#include "MapLayoutItemDelegate.h"

OwnerDelegate::OwnerDelegate(QWidget *parent = nullptr) : QStyledItemDelegate(parent) {}

void OwnerDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    
    auto color = index.data(Qt::UserRole).value<QColor>();

    if(color.isValid()) {
        
        //define colors
        painter->setBrush(QBrush(color, Qt::SolidPattern));

        //define size
        QRect indicator(
            QPoint(option.rect.left()+4, option.rect.top()+4),
            QSize(6,6)
        );

        //draw
        return painter->drawRect(indicator);
    }

    QStyledItemDelegate::paint(painter, option, index);
}

QSize OwnerDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    return QStyledItemDelegate::sizeHint(option, index);
}

LockAndVisibilityDelegate::LockAndVisibilityDelegate(QWidget *parent = nullptr) : QStyledItemDelegate(parent) { }

void LockAndVisibilityDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    
    //fetch data
    auto isHidden = index.data(VisibilityRole).toBool();
    auto isLocked = index.data(AvailabilityRole).toBool();

    if(isHidden) painter->drawPixmap(option.rect.topLeft(), QPixmap(":/icons/app/tools/hidden.png"));
    if(isLocked) painter->drawPixmap(QPoint(option.rect.right()-16, option.rect.top()), QPixmap(":/icons/app/tools/lock.png"));

}

QSize LockAndVisibilityDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    return QSize(32, 16);
}
