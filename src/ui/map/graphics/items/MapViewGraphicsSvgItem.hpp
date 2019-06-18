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

class MapViewGraphicsSvgItem : public QGraphicsSvgItem, public MapViewItemsNotifier {
    public:
        MapViewGraphicsSvgItem(MapViewItemsNotified* toNotify, const QString &fileName) : 
        QGraphicsSvgItem(fileName), 
        MapViewItemsNotifier(toNotify, this) {}
    
    private:
        QVariant itemChange(GraphicsItemChange change, const QVariant & value) override {
            MapViewItemsNotifier::_notifyItemChange(change);
            return QGraphicsSvgItem::itemChange(change, value);
        }
};