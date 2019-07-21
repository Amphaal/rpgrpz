#pragma once

#include <QGraphicsItem>
#include <QGraphicsPathItem>
#include <QGraphicsRectItem>
#include <QGraphicsSvgItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>

#include "src/ui/map/graphics/MapViewItemsNotifier.h"

#include "src/shared/models/AssetMetadata.h"

#include <QVariant>
#include <QBrush>
#include <QFont>
#include <QPixmapCache>

class MapViewGraphicsPixmapItem : public QGraphicsPixmapItem, public MapViewItemsNotifier {
    
    public:
        MapViewGraphicsPixmapItem(MapViewItemsNotified* toNotify, AssetMetadata &assetMetadata) : 
            QGraphicsPixmapItem(fetchCachedPixmap(assetMetadata)), 
            MapViewItemsNotifier(toNotify, this) {

        }

    private:
        QVariant itemChange(GraphicsItemChange change, const QVariant & value) override {
            MapViewItemsNotifier::_notifyItemChange(change);
            return QGraphicsPixmapItem::itemChange(change, value);
        }

        static QPixmap fetchCachedPixmap(AssetMetadata &assetMetadata) {
            QPixmap cached;

            auto assetId = assetMetadata.assetId();
            auto foundInCache = QPixmapCache::find(assetId, &cached);
            if(!foundInCache) {
                cached = QPixmap(assetMetadata.pathToAssetFile());
                QPixmapCache::insert(assetId, cached);
            }

            return cached;
        }
};