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

class MapViewGraphicsPixmapItem : public QGraphicsPixmapItem {
    
    public:
        MapViewGraphicsPixmapItem(const RPZAsset &assetMetadata) : 
            QGraphicsPixmapItem(*fetchCachedPixmap(assetMetadata)) { }

    private:

        static const QPixmap* fetchCachedPixmap(const RPZAsset &assetMetadata) {
            QPixmap* cached = nullptr;

            auto hash = assetMetadata.hash();
            auto foundInCache = QPixmapCache::find(hash, cached);
            if(!foundInCache) {
                QPixmap pix(assetMetadata.filepath());
                QPixmapCache::insert(hash, pix);
            }

            return cached;
        }
};