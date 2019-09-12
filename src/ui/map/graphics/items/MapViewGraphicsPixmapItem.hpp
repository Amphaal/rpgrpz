#pragma once

#include <QGraphicsItem>
#include <QGraphicsPathItem>
#include <QGraphicsRectItem>
#include <QGraphicsSvgItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>

#include "src/ui/map/graphics/GraphicsItemsChangeNotifier.h"

#include "src/shared/models/RPZToyMetadata.h"

#include <QVariant>
#include <QBrush>
#include <QFont>
#include <QPixmapCache>

class MapViewGraphicsPixmapItem : public QGraphicsPixmapItem, public GraphicsItemsChangeNotifier {
    
    public:
        MapViewGraphicsPixmapItem(const RPZToyMetadata &assetMetadata) : 
            QGraphicsPixmapItem(fetchCachedPixmap(assetMetadata)), 
            GraphicsItemsChangeNotifier(this) { }

    private:
        QVariant itemChange(GraphicsItemChange change, const QVariant &value) override {
            GraphicsItemsChangeNotifier::_notifyItemChange(change);
            return QGraphicsPixmapItem::itemChange(change, value);
        }

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