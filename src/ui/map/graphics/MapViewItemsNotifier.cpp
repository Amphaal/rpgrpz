#include "MapViewItemsNotifier.h"

MapViewItemsNotifier::~MapViewItemsNotifier() {
    if(this->_debugPoint) delete this->_debugPoint;
}

QFlags<QGraphicsItem::GraphicsItemFlag> MapViewItemsNotifier::defaultFlags() {
    return QFlags<QGraphicsItem::GraphicsItemFlag>(
        QGraphicsItem::GraphicsItemFlag::ItemIsSelectable |
        QGraphicsItem::GraphicsItemFlag::ItemIsMovable
    );
}

MapViewItemsNotifier::MapViewItemsNotifier(MapViewItemsNotified* targetToNotify, QGraphicsItem* item) : _toNotify(targetToNotify), _item(item) { }

void MapViewItemsNotifier::disableNotifications() {

    auto def = defaultFlags();
    this->_item->setFlags(def);

    this->_mustNotify = false;
}

void MapViewItemsNotifier::activateNotifications() {

    auto flags = defaultFlags();
    flags.setFlag(QGraphicsItem::GraphicsItemFlag::ItemSendsGeometryChanges);
    this->_item->setFlags(flags);

    this->_mustNotify = true;
}


void MapViewItemsNotifier::_notifyItemChange(int change) {
    if(!this->_item->scene()) return;
    if(!this->_mustNotify) return;
    
    // this->_updateDebugPoint();

    switch(change) {
        case QGraphicsItem::ItemPositionHasChanged: {
            this->_toNotify->onGraphicsItemCustomChange(this->_item, MapViewCustomItemsEventFlag::Moved);
        }
        break;
    }
}

void MapViewItemsNotifier::_updateDebugPoint() {
    
    if(this->_debugPoint) delete this->_debugPoint;

    auto pos = this->_item->pos();
    auto line = QLineF(
        pos,
        pos + QPointF(.01, .01)
    );

    QPen pen;
    pen.setWidth(1);
    pen.setColor("#FF0000");

    this->_debugPoint = this->_item->scene()->addLine(line, pen);

}