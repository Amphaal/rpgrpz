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

#include "src/shared/models/_base/RPZMap.hpp"
#include "src/shared/models/RPZAtom.h"

#include "AtomRelatedPayload.hpp"

#include "src/shared/database/MapDatabase.h"

class AtomsWielderPayload : public AtomRelatedPayload {
 public:
    AtomsWielderPayload() {}

    const QSet<RPZAsset::Hash> assetHashes() const {
        QSet<RPZAsset::Hash> out;

        auto hashes = this->value(QStringLiteral(u"assets")).toList();

        for (const auto &i : hashes) out += i.toString();

        return out;
    }

    const RPZMap<RPZAtom> atoms() const {
        RPZMap<RPZAtom> out;

        auto atoms = this->value(QStringLiteral(u"atoms")).toMap();

        for (auto i = atoms.begin(); i != atoms.end(); ++i) {
            auto snowflakeId = i.key().toULongLong();
            RPZAtom atom(i.value().toHash());
            out.insert(snowflakeId, atom);
        }

        return out;
    }

    AtomRelatedPayload::RemainingAtomIds restrictTargetedAtoms(const QSet<RPZAtom::Id> &idsToRemove) override {
        auto remainingAtoms = this->atoms();

        for (const auto &id : idsToRemove) {
            remainingAtoms.remove(id);
        }

        this->_setAtoms(remainingAtoms);
        this->_setAssetHashes(remainingAtoms);

        return remainingAtoms.count();
    };

 protected:
    explicit AtomsWielderPayload(const QVariantHash &hash) : AtomRelatedPayload(hash) {}
    explicit AtomsWielderPayload(const MapDatabase &map) : AtomRelatedPayload(Payload::Alteration::Reset) {
        this->_setAssetHashes(map.usedAssetHashes());
        this->_setAtoms(map.atoms().toVMap());
    }

    explicit AtomsWielderPayload(const QList<RPZAtom> &atoms) : AtomRelatedPayload(Payload::Alteration::Added) {
        this->_setAssetHashes(atoms);
        this->_setAtoms(atoms);
    }

 private:
    void _setAssetHashes(const QList<RPZAtom> &toExtractFrom) {
        QVariantList vList;

        for (auto &atom : toExtractFrom) {
            vList += atom.assetHash();
        }

        this->_setAssetHashes(vList);
    }

    void _setAssetHashes(const RPZMap<RPZAtom> &toExtractFrom) {
        QVariantList vList;

        for (auto &atom : toExtractFrom) {
            vList += atom.assetHash();
        }

        this->_setAssetHashes(vList);
    }


    void _setAssetHashes(const QSet<RPZAsset::Hash> &hashes) {
        QVariantList vList;

        for (auto &hash : hashes) {
            vList += hash;
        }

        this->_setAssetHashes(vList);
    }

    void _setAtoms(const RPZMap<RPZAtom> &atoms) {
        QVariantMap vMap;

        for (auto &atom : atoms) {
            vMap.insert(atom.idAsStr(), atom);
        }

        this->_setAtoms(vMap);
    }

    void _setAtoms(const QList<RPZAtom> &atoms) {
        QVariantMap vMap;

        for (auto &atom : atoms) {
            vMap.insert(atom.idAsStr(), atom);
        }

        this->_setAtoms(vMap);
    }

    void _setAssetHashes(const QVariantList &hashes) {
        this->insert(QStringLiteral(u"assets"), hashes);
    }

    void _setAtoms(const QVariantMap &atoms) {
        this->insert(QStringLiteral(u"atoms"), atoms);
    }
};
