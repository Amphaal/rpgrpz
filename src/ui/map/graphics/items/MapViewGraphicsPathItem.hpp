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

class MapViewGraphicsPathItem : public QGraphicsPathItem, public GraphicsItemsChangeNotifier {
    public:
        MapViewGraphicsPathItem(const QPainterPath & path, const QPen &pen, const QBrush &brush = QBrush()) : 
        QGraphicsPathItem(path), 
        GraphicsItemsChangeNotifier(this),
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
            GraphicsItemsChangeNotifier::_notifyItemChange(change);
            return QGraphicsPathItem::itemChange(change, value);
        }
};