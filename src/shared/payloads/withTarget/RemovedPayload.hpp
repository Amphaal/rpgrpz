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

#include "src/shared/payloads/_base/MultipleAtomTargetsPayload.hpp"

class RemovedPayload : public MultipleAtomTargetsPayload {
    public:
        explicit RemovedPayload(const QVariantHash &hash) : MultipleAtomTargetsPayload(hash) {}
        RemovedPayload(const QList<RPZAtom::Id> &atomIdsToRemove) : MultipleAtomTargetsPayload(Payload::Alteration::Removed, atomIdsToRemove) {
            Q_ASSERT(atomIdsToRemove.count() != 0);
            Q_ASSERT(atomIdsToRemove.first() != 0);
        }
};