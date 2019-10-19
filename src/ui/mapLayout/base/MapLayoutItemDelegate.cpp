#include "MapLayoutItemDelegate.h"

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
    return QSize(32, 16);
}
