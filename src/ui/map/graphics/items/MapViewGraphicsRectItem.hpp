#pragma once

#include <QGraphicsItem>
#include <QGraphicsPathItem>
#include <QGraphicsRectItem>
#include <QGraphicsSvgItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>

#include "src/ui/map/graphics/GraphicsItemsChangeNotifier.h"

#include <QVariant>
#include <QBrush>
#include <QFont>

class MapViewGraphicsRectItem : public QGraphicsRectItem, public GraphicsItemsChangeNotifier {
    public:
        MapViewGraphicsRectItem(const QRectF & rect, const QPen &pen, const QBrush &brush) :
        QGraphicsRectItem(rect), 
        GraphicsItemsChangeNotifier(this) {
            this->setBrush(brush);
            this->setPen(pen);
        }
            
    private:
        QVariant itemChange(GraphicsItemChange change, const QVariant & value) override {
            GraphicsItemsChangeNotifier::_notifyItemChange(change);
            return QGraphicsRectItem::itemChange(change, value);
        }
};