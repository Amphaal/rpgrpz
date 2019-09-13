#pragma once

#include "src/shared/models/RPZAtom.h"

#include <QPainterPath>
#include "src/ui/map/graphics/items/MapViewGraphicsItems.h"

enum class AtomConverterDataIndex { BrushTransform = 2555, IsTemporary = 2666, BrushDrawStyle = 2767 };

class AtomConverter {
    public:
        static void updateGraphicsItemFromAtom(QGraphicsItem* target, const RPZAtom &blueprint, bool isTargetTemporary = false);
        static void updateGraphicsItemFromMetadata(QGraphicsItem* item, const AtomParameter &param, const QVariant &val);
        static void updateGraphicsItemFromMetadata(QGraphicsItem* item, const AtomUpdates &updates);
        static RPZAtom graphicsToAtom(QGraphicsItem* blueprint, RPZAtom templateCopy);

    private:
        bool static _isTemporary(QGraphicsItem* item);
        void static _bulkTransformApply(QGraphicsItem* itemBrushToUpdate);
        bool static _setParamToGraphicsItemFromAtom(const AtomParameter &param, QGraphicsItem* itemToUpdate, const QVariant &val);
        void static _setParamToAtomFromGraphicsItem(const AtomParameter &param, RPZAtom &atomToUpdate, QGraphicsItem* blueprint);
};