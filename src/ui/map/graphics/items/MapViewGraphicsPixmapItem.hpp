#pragma once

#include <QGraphicsItem>
#include <QGraphicsPathItem>
#include <QGraphicsRectItem>
#include <QGraphicsSvgItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>

#include "src/shared/models/RPZToyMetadata.h"

#include <QVariant>
#include <QBrush>
#include <QFont>
#include <QPixmapCache>

class MapViewGraphicsPixmapItem : public QGraphicsPixmapItem {
    
    public:
        MapViewGraphicsPixmapItem(const RPZToyMetadata &assetMetadata) : 
            QGraphicsPixmapItem(fetchCachedPixmap(assetMetadata)) { }

    private:

        static QPixmap fetchCachedPixmap(const RPZToyMetadata &assetMetadata) {
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