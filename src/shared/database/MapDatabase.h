#pragma once

#include <QString>
#include <QVector>
#include "src/shared/models/RPZAtom.h"

#include "src/shared/models/_base/RPZMap.hpp"

#include "src/shared/database/_base/JSONDatabase.h"
#include "src/helpers/JSONSerializer.h"

class MapDatabase : public JSONDatabase {
    
    public:
        MapDatabase(const QString &filePath);
        MapDatabase(const QJsonObject &obj);
        MapDatabase();

        void saveIntoFile();

        void addAtom(const RPZAtom &toAdd);
        void addAtoms(const QList<RPZAtom> &toAdd);
        void updateAtom(const RPZAtomId &toUpdate, const AtomUpdates &updates);
        void updateAtom(const RPZAtom &updated);
        void removeAtom(const RPZAtomId &toRemove);
        void clear();

        const RPZAtom* atom(const RPZAtomId &id);
        const RPZAtom atomAsCopy(const RPZAtomId &id) const;
        RPZMap<RPZAtom>& atoms();
        const RPZMap<RPZAtom>& atoms() const;
        const QSet<RPZAssetHash>& usedAssetHashes() const;

    protected:
        const JSONDatabase::Version apiVersion() const override;

        RPZMap<RPZAtom> _atomsById;
        QSet<RPZAssetHash> _assetHashes;

    private:
        QHash<JSONDatabase::Version, JSONDatabase::UpdateHandler> _getUpdateHandlers() override;
        JSONDatabase::Model _getDatabaseModel() override;
        void _setupLocalData() override;

};