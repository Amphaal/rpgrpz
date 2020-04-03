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

#include "src/shared/models/toy/RPZAsset.hpp"
#include "src/shared/models/RPZAtom.h"

class RPZToy : public RPZAsset {
    public:
        RPZToy() {}
        explicit RPZToy(const QVariantHash &hash) : RPZAsset(hash) {}
        RPZToy(const RPZAsset &asset, const RPZAtom::Type &type = RPZAtom::Type::Undefined) : RPZAsset(asset) {
            this->insert(QStringLiteral(u"at"), (int)type);
        }

        RPZAtom::Type atomType() const {
            return (RPZAtom::Type)this->value(QStringLiteral(u"at")).toInt();
        }
};