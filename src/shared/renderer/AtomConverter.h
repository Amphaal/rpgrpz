#pragma once

#include <QPainterPath>

#include "src/shared/models/RPZAtom.h"

#include "src/shared/renderer/graphics/MapViewGraphics.h"
#include "src/shared/renderer/animator/MapViewAnimator.hpp"

#include "src/network/rpz/client/RPZClient.h"

class AtomConverter {
    public:   
        enum class DataIndex { BrushTransform = 2555, IsTemporary = 2666, BrushDrawStyle = 2767 };

            static QVariantHash brushTransform(QGraphicsItem *item);
            static void setBrushTransform(QGraphicsItem *item, const QVariantHash &transforms);

            static bool isTemporary(QGraphicsItem* item);
            static void setIsTemporary(QGraphicsItem* item, bool isTemporary);

            static RPZAtom::BrushType brushDrawStyle(QGraphicsItem* item);
            static void setBrushDrawStyle(QGraphicsItem* item, const RPZAtom::BrushType &style);

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
inline uint qHash(const AtomConverter::DataIndex &key, uint seed = 0) {return uint(key) ^ seed;}
