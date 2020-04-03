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

// Any graphical resources available within the source code may 
// use a different license and copyright : please refer to their metadata
// for further details. Graphical resources without explicit references to a
// different license and copyright still refer to this GNU General Public License.

#pragma once

#include "src/shared/payloads/_base/AlterationPayload.hpp"

#include "src/helpers/JSONSerializer.h"

class FogChangedPayload : public AlterationPayload {
    
    public:
        enum class ChangeType {
            Added,
            Removed,
            Reset
        };

        explicit FogChangedPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        FogChangedPayload(const ChangeType &type, const QList<QPolygonF> &modifyingPolys = QList<QPolygonF>()) : AlterationPayload(Payload::Alteration::FogChanged) {
            
            this->insert(QStringLiteral(u"ct"), (int)type);
            
            if(!modifyingPolys.isEmpty()) {
                this->insert(
                    QStringLiteral(u"mp"), 
                    JSONSerializer::fromPolygons(modifyingPolys)
                );
            }

        }
    
        ChangeType changeType() const {
            return (ChangeType)this->value(QStringLiteral(u"ct")).toInt();
        }

        const QList<QPolygonF> modifyingPolys() const {
            return JSONSerializer::toPolygons(
                this->value("mp").toList()
            );
        }
};