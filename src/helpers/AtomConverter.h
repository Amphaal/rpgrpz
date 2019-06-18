#pragma once

#include "src/shared/models/RPZAtom.h"

#include <QPainterPath>
#include "src/ui/map/graphics/MapViewGraphicsItem.hpp"

enum class AtomConverterDataIndex { TemplateAtom = 222, BrushTransform = 555, IsTemporary = 666, BrushDrawStyle = 767 };

class AtomConverter {
    
    public:
        static void updateGraphicsItemFromAtom(QGraphicsItem* target, RPZAtom &blueprint, bool isTargetTemporary = false);
        static void updateGraphicsItemFromMetadata(QGraphicsItem* item, const AtomParameter &param, QVariant &val);
        static RPZAtom graphicsToAtom(QGraphicsItem* blueprint);

    private:
        bool static _isTemporary(QGraphicsItem* item);
        void static _bulkTransformApply(QGraphicsItem* itemBrushToUpdate);
        bool static _setParamToGraphicsItemFromAtom(const AtomParameter &param, QGraphicsItem* itemToUpdate, QVariant &val);
        void static _setParamToAtomFromGraphicsItem(const AtomParameter &param, RPZAtom &atomToUpdate, QGraphicsItem* blueprint);
};