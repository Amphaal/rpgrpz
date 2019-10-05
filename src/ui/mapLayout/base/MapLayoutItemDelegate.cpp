#include "MapLayoutItemDelegate.h"

OwnerDelegate::OwnerDelegate(QWidget *parent) : QStyledItemDelegate(parent) {}

void OwnerDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {

    QStyledItemDelegate::paint(painter, option, index);

    auto color = index.data(RPZUserRoles::UserColor).value<QColor>();

    if(color.isValid()) {
        
        //define colors
        painter->setBrush(QBrush(color, Qt::SolidPattern));

        auto center = option.rect.center();
        center.setX(option.rect.right() - 8);

        //define size
        QRect indicator(
            QPoint(0,0),
            QSize(6,6)
        );
        indicator.moveCenter(center);

        //draw
        painter->drawRect(indicator);
    }

}

QSize OwnerDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    return QStyledItemDelegate::sizeHint(option, index);
    // return QSize(24, 12);
}

LockAndVisibilityDelegate::LockAndVisibilityDelegate(QWidget *parent) : QStyledItemDelegate(parent) {
    if(!_hiddenPix && !_lockPix) {
        _hiddenPix = new QPixmap(":/icons/app/tools/hidden.png");
        _lockPix = new QPixmap(":/icons/app/tools/lock.png");
    }
 }

void LockAndVisibilityDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {

    QStyledItemDelegate::paint(painter, option, index);

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
    //return QStyledItemDelegate::sizeHint(option, index);
    return QSize(32, 16);
}
