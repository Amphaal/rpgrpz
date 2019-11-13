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

#include "src/shared/renderer/graphics/_base/RPZGraphicsItem.hpp"

class MapViewGraphicsPixmapItem : public QObject, public QGraphicsPixmapItem, public RPZGraphicsItem {

    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

    public:
        MapViewGraphicsPixmapItem(const RPZAsset &assetMetadata) : QGraphicsPixmapItem(assetMetadata.filepath()) {}
        MapViewGraphicsPixmapItem(const QString &filepath) : QGraphicsPixmapItem(filepath) {}

    private:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override {
            auto result = this->conditionnalPaint(this, painter, option, widget);
            if(!result.mustContinue) return;
            QGraphicsPixmapItem::paint(painter, &result.options, widget);
        }
};