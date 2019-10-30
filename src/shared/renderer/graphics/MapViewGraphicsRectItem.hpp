#pragma once

#include <QGraphicsItem>
#include <QGraphicsPathItem>
#include <QGraphicsRectItem>
#include <QGraphicsSvgItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>

#include <QVariant>
#include <QBrush>
#include <QFont>

class MapViewGraphicsRectItem : public QGraphicsRectItem {
    public:
        MapViewGraphicsRectItem(const QRectF &rect, const QPen &pen, const QBrush &brush) :
        QGraphicsRectItem(rect) {
            this->setBrush(brush);
            this->setPen(pen);
        }

};