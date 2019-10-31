#pragma once

#include <QGraphicsItem>
#include <QGraphicsPathItem>
#include <QGraphicsRectItem>
#include <QGraphicsSvgItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>

#include "src/shared/models/toy/RPZAsset.hpp"

#include <QVariant>
#include <QBrush>
#include <QFont>
#include <QPixmapCache>
#include <QObject>

class MapViewGraphicsPixmapItem : public QObject, public QGraphicsPixmapItem {

    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

    public:
        MapViewGraphicsPixmapItem(const RPZAsset &assetMetadata) : QGraphicsPixmapItem(assetMetadata.filepath()) {}
};