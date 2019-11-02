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

#include <QObject>

class MapViewGraphicsRectItem : public QObject, public QGraphicsRectItem {

    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

    public:
        MapViewGraphicsRectItem(const QRectF &rect, const QPen &pen, const QBrush &brush) :
        QGraphicsRectItem(rect) {
            this->setBrush(brush);
            this->setPen(pen);
        }

};