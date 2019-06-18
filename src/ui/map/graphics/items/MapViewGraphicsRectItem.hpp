#pragma once

#include <QGraphicsItem>
#include <QGraphicsPathItem>
#include <QGraphicsRectItem>
#include <QGraphicsSvgItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>

#include "src/ui/map/graphics/MapViewItemsNotifier.h"

#include <QVariant>
#include <QBrush>
#include <QFont>

class MapViewGraphicsRectItem : public QGraphicsRectItem, public MapViewItemsNotifier {
    public:
        MapViewGraphicsRectItem(MapViewItemsNotified* toNotify, const QRectF & rect, const QPen &pen, const QBrush &brush) :
        QGraphicsRectItem(rect), 
        MapViewItemsNotifier(toNotify, this) {
            this->setBrush(brush);
            this->setPen(pen);
        }
            
    private:
        QVariant itemChange(GraphicsItemChange change, const QVariant & value) override {
            MapViewItemsNotifier::_notifyItemChange(change);
            return QGraphicsRectItem::itemChange(change, value);
        }
};