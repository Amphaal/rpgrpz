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
#include <QVector>
#include <QDateTime>

#include "src/shared/models/RPZAtom.h"
#include "src/shared/models/RPZMapParameters.hpp"
#include "src/shared/models/RPZFogParams.hpp"

#include "src/shared/models/_base/RPZMap.hpp"

#include "src/shared/database/_base/JSONDatabase.h"
#include "src/helpers/JSONSerializer.h"

#include "src/helpers/VectorSimplifier.hpp"
#include "src/shared/payloads/fog/FogChangedPayload.hpp"

class MapDatabase : public JSONDatabase {
 public:
    struct FogBuffer {
        QList<QPolygonF> polys;
        ClipperLib::Paths paths;
    };

    explicit MapDatabase(const QString &filePath);
    explicit MapDatabase(const QJsonObject &obj);
    MapDatabase();

    const QString snapshotSave(const QString &folderToSaveTo);

    void setMapParams(const RPZMapParameters &newParams);
    void setFogParams(const RPZFogParams &fogParams);
    QList<QPolygonF> alterFog(const FogChangedPayload &payload);
    void changeFogMode(const RPZFogParams::Mode &mode);

    void addAtom(const RPZAtom &toAdd);
    void addAtoms(const QList<RPZAtom> &toAdd);
    void updateAtom(const RPZAtom::Id &toUpdate, const RPZAtom::Updates &updates);
    void updateAtom(const RPZAtom &updated);
    void removeAtom(const RPZAtom::Id &toRemove);
    void clear();

    const RPZMapParameters mapParams() const;
    const RPZFogParams fogParams() const;

    const RPZAtom atom(const RPZAtom::Id &id) const;
    RPZAtom* atomPtr(const RPZAtom::Id &id);
    RPZMap<RPZAtom>& atoms();
    const RPZMap<RPZAtom>& atoms() const;
    const QSet<RPZAsset::Hash>& usedAssetHashes() const;

 protected:
    JSONDatabase::Version apiVersion() const override;
    const QJsonObject _updatedInnerDb() override;

    RPZMap<RPZAtom> _atomsById;
    QSet<RPZAsset::Hash> _assetHashes;
    RPZMapParameters _mapParams;
    RPZFogParams _fogParams;

 private:
    FogBuffer _fogBuffer;
    QHash<JSONDatabase::Version, JSONDatabase::UpdateHandler> _getUpdateHandlers() override;
    JSONDatabase::Model _getDatabaseModel() override;
    void _setupLocalData() override;
};
