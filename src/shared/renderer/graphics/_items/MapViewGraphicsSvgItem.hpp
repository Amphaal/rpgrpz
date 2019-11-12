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

class MapViewGraphicsSvgItem : public QGraphicsSvgItem {
    public:
        MapViewGraphicsSvgItem(const QString &fileName) : QGraphicsSvgItem(fileName) {}
    
};