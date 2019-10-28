#include "LockAndVisibilityDelegate.h"

LockAndVisibilityDelegate::LockAndVisibilityDelegate(QWidget *parent) : QStyledItemDelegate(parent) {
    if(!_hiddenPix && !_lockPix) {
        _hiddenPix = new QPixmap(":/icons/app/tools/hidden.png");
        _lockPix = new QPixmap(":/icons/app/tools/lock.png");
    }
 }

void LockAndVisibilityDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {

    QStyledItemDelegate::paint(painter, option, index);

    //fetch data
    auto item = MapLayoutItem::fromIndex(index);
    if(!item) return;

    //check if atom
    auto atom = dynamic_cast<MapLayoutAtom*>(item);
    if(!atom) return;

    //may draw "hide" icon
    if(atom->isHidden()) {
        painter->drawPixmap(option.rect.topLeft(), *_hiddenPix);
    }
    
    //may draw "lock" icon
    if(atom->isLocked()) {
        auto startPoint = QPoint(
            option.rect.right() - 16, 
            option.rect.top()
        );
        painter->drawPixmap(startPoint, *_lockPix);
    }

}

QSize LockAndVisibilityDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    return QSize(4, 4);
}
