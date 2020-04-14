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
// different license and copyright still refer to this GNU General Public License.

#pragma once

#include "src/shared/renderer/graphics/_generic/MapViewGraphicsPathItem.hpp"
#include "src/shared/models/RPZUser.h"

class QuickDrawItem : public MapViewGraphicsPathItem {
    public:
        using Id = SnowFlake::Id;

        QuickDrawItem(const QuickDrawItem::Id &id, const QColor &color, const QPainterPath &firstBits = QPainterPath()) : MapViewGraphicsPathItem(QPainterPath(), QPen(), QBrush()) {
            QPen pen;
            pen.setColor(color);
            this->setPen(pen);
            this->_id = id;
            this->setPath(firstBits);
        }
        QuickDrawItem(const RPZUser &emiter) : QuickDrawItem(SnowFlake::get()->nextId(), emiter.color()) {}

        QuickDrawItem::Id id() const {
            return this->_id;
        }

        void addPathBits(const QPainterPath &bits) {
            //TODO
        }
    
    private:
        QuickDrawItem() : MapViewGraphicsPathItem(QPainterPath(), QPen(), QBrush()) {
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, false);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsFocusable, false);
        }

        QuickDrawItem::Id _id;

};