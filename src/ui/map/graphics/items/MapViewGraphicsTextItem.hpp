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

class MapViewGraphicsTextItem : public QGraphicsTextItem, public GraphicsItemsChangeNotifier {
    public:
        MapViewGraphicsTextItem(const QString &text, int textSize) : 
        QGraphicsTextItem(text), 
        GraphicsItemsChangeNotifier(this) {
            
            auto font = QFont();
            font.setPointSize(textSize);
            this->setFont(font);
            
        }

    private:
        QVariant itemChange(GraphicsItemChange change, const QVariant & value) override {
            GraphicsItemsChangeNotifier::_notifyItemChange(change);
            return QGraphicsTextItem::itemChange(change, value);
        }

};