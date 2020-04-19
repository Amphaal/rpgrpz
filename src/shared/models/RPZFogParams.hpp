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

#include <QVariantHash>

#include "src/helpers/JSONSerializer.h"

class RPZFogParams : public QVariantHash {

 public:
        enum class Mode {
            PathIsFog,
            PathIsButFog
        };

        RPZFogParams() {};
        explicit RPZFogParams(const QVariantHash &hash) : QVariantHash(hash) {};

        Mode mode() const {
            return (Mode)this->value("fm").toInt();
        }

        void setMode(const RPZFogParams::Mode &mode) {
            this->insert("fm", (int)mode);
        }

        QList<QPolygonF> polys() const {
            return JSONSerializer::toPolygons(
                this->value("polys").toList()
            );
        }

        void setPolys(const QList<QPolygonF> &polys) {
            this->insert("polys", 
                JSONSerializer::fromPolygons(polys)
            );
        }

};