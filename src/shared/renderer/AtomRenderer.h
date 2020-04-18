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

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

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