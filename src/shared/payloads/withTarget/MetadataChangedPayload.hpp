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

#include <QString>

#include "src/shared/payloads/_base/MultipleAtomTargetsPayload.hpp"
#include "src/shared/models/RPZAtom.h"

class MetadataChangedPayload : public MultipleAtomTargetsPayload {
 public:
        explicit MetadataChangedPayload(const QVariantHash &hash) : MultipleAtomTargetsPayload(hash) {}
        MetadataChangedPayload(const QList<RPZAtom::Id> &targetedRPZAtomIds, const RPZAtom::Updates &changes) : MultipleAtomTargetsPayload(Payload::Alteration::MetadataChanged, targetedRPZAtomIds) {
            Q_ASSERT(targetedRPZAtomIds.count());
            Q_ASSERT(changes.count());

            this->_setMetadataChanges(changes);
        }

        RPZAtom::Updates updates() const {
            return RPZAtom::unserializeUpdates(this->args().toHash());
        }

 private:
        void _setMetadataChanges(const RPZAtom::Updates &changes) {
            this->insert(QStringLiteral(u"args"),
                RPZAtom::serializeUpdates(changes)
            );
        }
};
