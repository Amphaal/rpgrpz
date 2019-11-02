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

class MapViewGraphicsPathItem : public QObject, public QGraphicsPathItem {

    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
    Q_PROPERTY(QPainterPath path READ path WRITE setPath)

    public:
        MapViewGraphicsPathItem(const QPainterPath &path, const QPen &pen, const QBrush &brush = QBrush()) : 
        QGraphicsPathItem(path),
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

};