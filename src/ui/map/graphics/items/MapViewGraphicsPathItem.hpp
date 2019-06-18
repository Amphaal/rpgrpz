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

class MapViewGraphicsPathItem : public QGraphicsPathItem, public MapViewItemsNotifier {
    public:
        MapViewGraphicsPathItem(MapViewItemsNotified* toNotify, const QPainterPath & path, const QPen &pen, const QBrush &brush = QBrush()) : 
        QGraphicsPathItem(path), 
        MapViewItemsNotifier(toNotify, this),
        _sourceBrush(brush) {
            this->_sourceBrushSize = QSizeF(this->_sourceBrush.texture().size());
            this->setPen(pen);
        }

        QBrush sourceBrush() {
            return this->_sourceBrush;
        }

        QSizeF sourceBrushSize() {
            return this->_sourceBrushSize;
        }

    private:
        QBrush _sourceBrush;
        QSizeF _sourceBrushSize;

        QVariant itemChange(GraphicsItemChange change, const QVariant & value) override {
            MapViewItemsNotifier::_notifyItemChange(change);
            return QGraphicsPathItem::itemChange(change, value);
        }
};