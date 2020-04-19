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

#include "src/shared/payloads/_base/AtomRelatedPayload.hpp"

class BulkMetadataChangedPayload : public AtomRelatedPayload {
 public:
    explicit BulkMetadataChangedPayload(const QVariantHash &hash) : AtomRelatedPayload(hash) { }
    explicit BulkMetadataChangedPayload(const RPZAtom::ManyUpdates &changes) : AtomRelatedPayload(Payload::Alteration::BulkMetadataChanged) {
        this->_defineAtomsUpdates(changes);
    }

    RPZAtom::ManyUpdates atomsUpdates() const {
        auto rawData = this->value(QStringLiteral(u"changes")).toHash();
        RPZAtom::ManyUpdates out;

        for (auto i = rawData.constBegin(); i != rawData.constEnd(); i++) {
            out.insert(
                i.key().toULongLong(),
                RPZAtom::unserializeUpdates(i.value().toHash())
            );
        }

        return out;
    }

    AtomRelatedPayload::RemainingAtomIds restrictTargetedAtoms(const QSet<RPZAtom::Id> &idsToRemove) override {
        auto remainingUpdates = this->atomsUpdates();

        for (const auto &id : idsToRemove) {
            remainingUpdates.remove(id);
        }

        this->_defineAtomsUpdates(remainingUpdates);

        return remainingUpdates.count();
    };

 private:
    void _defineAtomsUpdates(const RPZAtom::ManyUpdates &changes) {
        QVariantHash hash;

        for (auto i = changes.constBegin(); i != changes.constEnd(); i++) {
            auto idStr = QString::number(i.key());
            QVariantHash changesById;

            hash.insert(
                idStr,
                RPZAtom::serializeUpdates(i.value())
            );
        }

        this->insert(QStringLiteral(u"changes"), hash);
    }
};
