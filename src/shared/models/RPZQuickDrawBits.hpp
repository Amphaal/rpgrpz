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
// different license and copyright still refer to this GPL.

#pragma once

#include <QVariantHash>
#include <QPainterPath>

#include "src/helpers/JSONSerializer.h"
#include "src/shared/models/RPZUser.h"

class RPZQuickDrawBits : public QVariantHash {
 public:
        using Id = SnowFlake::Id;

        RPZQuickDrawBits() {}
        explicit RPZQuickDrawBits(const QVariantHash &hash) : QVariantHash(hash) {}
        RPZQuickDrawBits(const QPointF &scenePos, const RPZQuickDrawBits::Id &id, const RPZUser::Id &drawerId, const QPainterPath &bits, bool areLastBits) {
            this->insert("pos", JSONSerializer::fromPointF(scenePos));
            this->insert("id", QString::number(id));  // must be saved as string to prevent JSON parser lack of precision on double conversion
            this->insert("drwr_id", QString::number(drawerId));  // must be saved as string to prevent JSON parser lack of precision on double conversion
            this->insert("bits", JSONSerializer::asBase64(bits));
            this->insert("end", areLastBits);
        }

        RPZQuickDrawBits::Id drawId() const {
            return this->value("id").toULongLong();
        }

        QPointF scenePos() const {
            return JSONSerializer::toPointF(this->value("pos").toList());
        }

        RPZUser::Id drawerId() const {
            return this->value("drwr_id").toULongLong();
        }

        const QPainterPath bitsAsPath() const {
            return JSONSerializer::toPainterPath(this->value("bits").toByteArray());
        }

        bool areLastBits() const {
            return this->value("end").toBool();
        }
};
