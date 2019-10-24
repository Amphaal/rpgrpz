#pragma once

#include "src/ui/map/graphics/items/MapViewGraphicsItems.h"

#include "src/shared/models/toy/RPZToy.hpp"

#include "src/helpers/AtomConverter.h"

#include <QApplication>
#include <QFileInfo>

class CustomGraphicsItemHelper {
    public:
        static QGraphicsItem* createGraphicsItem(const RPZAtom &atom, const RPZAsset &asset, bool isTemporary = false);
        static QGraphicsRectItem* createMissingAssetPlaceholderItem(const RPZAtom &atom);
        static QGraphicsItem* createOutlineRectItem(const QPointF &scenePos);

    private:
        static QGraphicsItem* _createGenericImageBasedItem(const RPZAtom &atom, const RPZAsset &asset);
        static QGraphicsPathItem* _createBrushItem(const RPZAtom &atom, const RPZAsset &asset);
        static QGraphicsPathItem* _createDrawingItem(const RPZAtom &atom);
        static QGraphicsTextItem* _createTextItem(const RPZAtom &atom);
};