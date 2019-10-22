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

        virtual void addAtom(const RPZAtom &toAdd);
        void addAtoms(const QList<RPZAtom> &toAdd);
        void updateAtom(const RPZAtomId &toUpdate, const AtomUpdates &updates);
        void updateAtom(const RPZAtom &updated);
        void removeAtom(const RPZAtomId &toRemove);
        void clear();

        virtual const RPZMap<RPZAtom> safe_atoms() const;
        virtual const QSet<RPZAssetHash> safe_usedAssetsIds() const;

    protected:
        void saveIntoFile();
        const int apiVersion() override;

        const RPZMap<RPZAtom>& _atoms() const;
        const QSet<RPZAssetHash>& _usedAssetsIds() const;

    private:
        RPZMap<RPZAtom> _atomsById;
        QSet<RPZAssetHash> _assetHashes;

        QHash<JSONDatabase::Version, JSONDatabase::UpdateHandler> _getUpdateHandlers() override;
        JSONDatabase::Model _getDatabaseModel() override;
        void _setupLocalData() override;

};