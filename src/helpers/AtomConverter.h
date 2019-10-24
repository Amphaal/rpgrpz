#pragma once

#include "src/shared/models/RPZAtom.h"

#include <QPainterPath>
#include "src/ui/map/graphics/items/MapViewGraphicsItems.h"

class AtomConverter {
    public:   
        enum class DataIndex { BrushTransform = 2555, IsTemporary = 2666, BrushDrawStyle = 2767 };

            static QVariantHash brushTransform(QGraphicsItem *item);
            static void setBrushTransform(QGraphicsItem *item, const QVariantHash &transforms);

            static bool isTemporary(QGraphicsItem* item);
            static void setIsTemporary(QGraphicsItem* item, bool isTemporary);

            static BrushType brushDrawStyle(QGraphicsItem* item);
            static void setBrushDrawStyle(QGraphicsItem* item, const BrushType &style);

        static void updateGraphicsItemFromAtom(QGraphicsItem* target, const RPZAtom &blueprint, bool isTargetTemporary = false);
        static void updateGraphicsItemFromMetadata(QGraphicsItem* item, const AtomParameter &param, const QVariant &val);
        static void updateGraphicsItemFromMetadata(QGraphicsItem* item, const AtomUpdates &updates);
        static RPZAtom graphicsToAtom(QGraphicsItem* blueprint, RPZAtom templateCopy);

    private:
        static void _bulkTransformApply(QGraphicsItem* itemBrushToUpdate);
        static bool _setParamToGraphicsItemFromAtom(const AtomParameter &param, QGraphicsItem* itemToUpdate, const QVariant &val);
        static void _setParamToAtomFromGraphicsItem(const AtomParameter &param, RPZAtom &atomToUpdate, QGraphicsItem* blueprint);

};
inline uint qHash(const AtomConverter::DataIndex &key, uint seed = 0) {return uint(key) ^ seed;}
