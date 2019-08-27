#pragma once

#include "items/MapViewGraphicsItems.h"

#include "src/shared/models/RPZAtom.h"
#include "src/shared/models/RPZAssetMetadata.h"

#include "src/helpers/AtomConverter.h"

class CustomGraphicsItemHelper {
    public:
        static QGraphicsItem* createGraphicsItem(const RPZAtom &atom, const RPZAssetMetadata &assetMetadata, bool isTemporary = false);
        static QGraphicsRectItem* createMissingAssetPlaceholderItem(const RPZAtom &atom);
        static QGraphicsItem* createOutlineRectItem(const QPointF &scenePos);

    private:
        static QGraphicsItem* _createGenericImageBasedItem(const RPZAtom &atom, const RPZAssetMetadata &assetMetadata);
        static QGraphicsPathItem* _createBrushItem(const RPZAtom &atom, const RPZAssetMetadata &assetMetadata);
        static QGraphicsPathItem* _createDrawingItem(const RPZAtom &atom);
        static QGraphicsTextItem* _createTextItem(const RPZAtom &atom);
};