// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#pragma once

#include <QPainterPath>

#include "src/shared/models/RPZAtom.h"

#include "src/shared/renderer/graphics/MapViewGraphics.h"
#include "src/shared/renderer/animator/MapViewAnimator.hpp"

#include "src/helpers/RPZQVariant.hpp"

class AtomConverter {
    public:   
        static void updateGraphicsItemFromMetadata(QGraphicsItem* item, const RPZAtom::Updates &updates);
        static void setupGraphicsItemFromAtom(QGraphicsItem* target, const RPZAtom &blueprint);
        static const RPZAtom cloneAtomTemplateFromGraphics(QGraphicsItem* blueprint, RPZAtom toClone);

    private:
        static void _bulkTransformApply(QGraphicsItem* itemBrushToUpdate);
        static bool _setParamToGraphicsItemFromAtom(const RPZAtom::Parameter &param, QGraphicsItem* itemToUpdate, const QVariant &val);
        static void _updateAtomParamFromGraphics(const RPZAtom::Parameter &param, RPZAtom &atomToUpdate, QGraphicsItem* blueprint);

        static void _updateGraphicsItemFromMetadata(QGraphicsItem* item, const RPZAtom::Parameter &param, const QVariant &val);
        static void _updateGraphicsItemFromMetadata(QGraphicsItem* item, const RPZAtom &blueprint);

};

