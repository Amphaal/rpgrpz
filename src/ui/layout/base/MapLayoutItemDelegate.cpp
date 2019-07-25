#include "MapLayoutItemDelegate.h"

OwnerDelegate::OwnerDelegate(QWidget *parent) : QStyledItemDelegate(parent) {}

void OwnerDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    
    auto color = index.data(RPZUserRoles::UserColor).value<QColor>();

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
    return QSize(6, 6);
}

LockAndVisibilityDelegate::LockAndVisibilityDelegate(QWidget *parent) : QStyledItemDelegate(parent) {
    if(!_hiddenPix && !_lockPix) {
        _hiddenPix = new QPixmap(":/icons/app/tools/hidden.png");
        _lockPix = new QPixmap(":/icons/app/tools/lock.png");
    }
 }

void LockAndVisibilityDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    
    //fetch data
    auto isHidden = index.data(RPZUserRoles::AtomVisibility).toBool();
    auto isLocked = index.data(RPZUserRoles::AtomAvailability).toBool();

    //may draw "hide" icon
    if(isHidden) painter->drawPixmap(option.rect.topLeft(), *_hiddenPix);
    
    //may draw "lock" icon
    if(isLocked) {
        auto startPoint = QPoint(
            option.rect.right() - 16, 
            option.rect.top()
        );
        painter->drawPixmap(startPoint, *_lockPix);
    }

}

QSize LockAndVisibilityDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    return QSize(32, 16);
}
