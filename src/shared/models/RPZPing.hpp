// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2021 Guillaume Vara <guillaume.vara@gmail.com>

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

#include <QVariantHash>
#include <QPainterPath>

#include "src/helpers/JSONSerializer.h"
#include "src/shared/models/RPZUser.h"

class RPZPing : public QVariantHash {
 public:
        RPZPing() {}
        explicit RPZPing(const QVariantHash &hash) : QVariantHash(hash) {}
        RPZPing(const QPointF &scenePos, const RPZUser::Id &emiterId) {
            this->insert("pos", JSONSerializer::fromPointF(scenePos));
            this->insert("emiter_id", QString::number(emiterId));  // must be saved as string to prevent JSON parser lack of precision on double conversion
        }
        QPointF scenePos() const {
            return JSONSerializer::toPointF(this->value("pos").toList());
        }

        RPZUser::Id emiterId() const {
            return this->value("emiter_id").toULongLong();
        }
};
