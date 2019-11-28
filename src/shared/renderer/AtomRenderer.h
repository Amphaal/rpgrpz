#pragma once

#include "AtomConverter.h"

#include "src/shared/renderer/graphics/MapViewGraphics.h"

#include "src/shared/models/toy/RPZToy.hpp"

#include <QApplication>
#include <QFileInfo>

#include "src/helpers/RPZQVariant.hpp"

class AtomRenderer {
    public:
        static void defineMapParameters(const RPZMapParameters &mapParameters);

        static QGraphicsItem* createGraphicsItem(const RPZAtom &atom, const RPZAsset &asset, bool isTemporary, bool owned);
        static QGraphicsRectItem* createMissingAssetPlaceholderItem(const RPZAtom &atom);
        static QGraphicsItem* createOutlineRectItem(const QPointF &scenePos);

    private:
        static QGraphicsItem* _createGenericImageBasedItem(const RPZAtom &atom, const RPZAsset &asset);
        static QGraphicsPathItem* _createBrushItem(const RPZAtom &atom, const RPZAsset &asset);
        static MapViewDrawing* _createDrawingItem(const RPZAtom &atom);
        static QGraphicsTextItem* _createTextItem(const RPZAtom &atom);
        static MapViewToken* _createToken(const RPZAtom &atom, bool owned);

        static inline RPZMapParameters _mapParams;
};