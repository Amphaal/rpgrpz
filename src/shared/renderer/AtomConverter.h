#pragma once

#include <QPainterPath>

#include "src/shared/models/RPZAtom.h"

#include "src/shared/renderer/graphics/MapViewGraphics.h"
#include "src/shared/renderer/animator/MapViewAnimator.hpp"

#include "src/network/rpz/client/RPZClient.h"
#include "src/helpers/RPZQVariant.hpp"

class AtomConverter {
    public:   
        static void updateGraphicsItemFromMetadata(QGraphicsItem* item, const RPZAtom::Updates &updates);
        static void setupGraphicsItemFromAtom(QGraphicsItem* target, const RPZAtom &blueprint, bool isTargetTemporary = false);

        static RPZAtom graphicsToAtom(QGraphicsItem* blueprint, RPZAtom templateCopy);

    private:
        static void _bulkTransformApply(QGraphicsItem* itemBrushToUpdate);
        static bool _setParamToGraphicsItemFromAtom(const RPZAtom::Parameter &param, QGraphicsItem* itemToUpdate, const QVariant &val);
        static void _setParamToAtomFromGraphicsItem(const RPZAtom::Parameter &param, RPZAtom &atomToUpdate, QGraphicsItem* blueprint);

        static void _updateGraphicsItemFromMetadata(QGraphicsItem* item, const RPZAtom::Parameter &param, const QVariant &val);
        static void _updateGraphicsItemFromMetadata(QGraphicsItem* item, const RPZAtom &blueprint);

};

