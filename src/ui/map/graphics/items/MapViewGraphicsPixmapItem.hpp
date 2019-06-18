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

class MapViewGraphicsPixmapItem : public QGraphicsPixmapItem, public MapViewItemsNotifier {
    public:
        MapViewGraphicsPixmapItem(MapViewItemsNotified* toNotify, const QString &fileName) : 
        QGraphicsPixmapItem(QPixmap(fileName)), 
        MapViewItemsNotifier(toNotify, this) {}

    private:
        QVariant itemChange(GraphicsItemChange change, const QVariant & value) override {
            MapViewItemsNotifier::_notifyItemChange(change);
            return QGraphicsPixmapItem::itemChange(change, value);
        }
};