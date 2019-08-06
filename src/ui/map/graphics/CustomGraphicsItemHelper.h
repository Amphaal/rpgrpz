#pragma once

#include "items/MapViewGraphicsItems.h"

#include "src/shared/models/RPZAtom.h"
#include "src/shared/models/RPZAssetMetadata.h"

#include "src/helpers/AtomConverter.h"

class CustomGraphicsItemHelper {
    public:
        static QGraphicsItem* createGraphicsItem(RPZAtom &atom, const RPZAssetMetadata &assetMetadata, bool isTemporary = false);
        static QGraphicsRectItem* createMissingAssetPlaceholderItem(RPZAtom &atom);
        static QGraphicsItem* createOutlineRectItem(QPointF &scenePos);

    private:
        static QGraphicsItem* _createGenericImageBasedItem(RPZAtom &atom, const RPZAssetMetadata &assetMetadata);
        static QGraphicsPathItem* _createBrushItem(RPZAtom &atom, const RPZAssetMetadata &assetMetadata);
        static QGraphicsPathItem* _createDrawingItem(RPZAtom &atom);
        static QGraphicsTextItem* _createTextItem(RPZAtom &atom);
};