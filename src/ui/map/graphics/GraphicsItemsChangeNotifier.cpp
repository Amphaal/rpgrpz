#include "GraphicsItemsChangeNotifier.h"

GraphicsItemsChangeNotifier::~GraphicsItemsChangeNotifier() { }

QFlags<QGraphicsItem::GraphicsItemFlag> GraphicsItemsChangeNotifier::defaultFlags() {
    return QFlags<QGraphicsItem::GraphicsItemFlag>(
        QGraphicsItem::GraphicsItemFlag::ItemIsSelectable |
        QGraphicsItem::GraphicsItemFlag::ItemIsMovable
    );
}


GraphicsItemsChangeNotifier::GraphicsItemsChangeNotifier(QGraphicsItem* item) : _item(item) { }

void GraphicsItemsChangeNotifier::disableNotifications() {

    auto def = defaultFlags();
    this->_item->setFlags(def);

    this->_mustNotify = false;
}

void GraphicsItemsChangeNotifier::addNotified(ItemChangedNotified* notified) {
    this->_toNotify = notified;
}

QGraphicsItem* GraphicsItemsChangeNotifier::graphicsItem() {
    return this->_item;
}

void GraphicsItemsChangeNotifier::activateNotifications() {

    auto flags = defaultFlags();
    flags.setFlag(QGraphicsItem::GraphicsItemFlag::ItemSendsGeometryChanges);
    this->_item->setFlags(flags);

    this->_mustNotify = true;
}


void GraphicsItemsChangeNotifier::_notifyItemChange(int change) {
    if(!this->_item->scene()) return;
    if(!this->_mustNotify) return;
    if(!this->_toNotify) return;

    switch(change) {
        case QGraphicsItem::ItemPositionHasChanged: {
            this->_toNotify->onItemChanged(this, MapViewCustomItemsEventFlag::Moved);
        }
        break;
    }
}