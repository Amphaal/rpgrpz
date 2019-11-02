#pragma once

#include "AtomConverter.h"

#include "src/shared/renderer/graphics/MapViewGraphics.h"

#include "src/shared/models/toy/RPZToy.hpp"

#include <QApplication>
#include <QFileInfo>

class AtomRenderer {
    public:
        static QGraphicsItem* createGraphicsItem(const RPZAtom &atom, const RPZAsset &asset, const QSizeF &tileSize, bool isTemporary = false);
        static QGraphicsRectItem* createMissingAssetPlaceholderItem(const RPZAtom &atom);
        static QGraphicsItem* createOutlineRectItem(const QPointF &scenePos);

    private:
        static QGraphicsItem* _createGenericImageBasedItem(const RPZAtom &atom, const RPZAsset &asset);
        static QGraphicsPathItem* _createBrushItem(const RPZAtom &atom, const RPZAsset &asset);
        static QGraphicsPathItem* _createDrawingItem(const RPZAtom &atom);
        static QGraphicsTextItem* _createTextItem(const RPZAtom &atom);
        static QGraphicsPixmapItem* _createUnscalableToken(const RPZAtom &atom);
        static MapViewToken* _createPlayerToken(const RPZAtom &atom, const QSizeF &tileSize);
};