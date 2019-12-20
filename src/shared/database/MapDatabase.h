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
#include "src/helpers/VectorSimplifier.hpp"

class MapDatabase : public JSONDatabase {
    
    public:
        struct FogBuffer {
            QList<QPolygonF> polys;
            ClipperLib::Paths paths;
        };

        MapDatabase(const QString &filePath);
        MapDatabase(const QJsonObject &obj);
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
        const JSONDatabase::Version apiVersion() const override;
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