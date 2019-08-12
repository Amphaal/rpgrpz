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

class MapViewGraphicsSvgItem : public QGraphicsSvgItem, public GraphicsItemsChangeNotifier {
    public:
        MapViewGraphicsSvgItem(const QString &fileName) : 
        QGraphicsSvgItem(fileName), 
        GraphicsItemsChangeNotifier(this) {}
    
    private:
        QVariant itemChange(GraphicsItemChange change, const QVariant & value) override {
            GraphicsItemsChangeNotifier::_notifyItemChange(change);
            return QGraphicsSvgItem::itemChange(change, value);
        }
};