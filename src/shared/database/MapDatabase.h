#pragma once

#include <QString>
#include <QVector>
#include "src/shared/models/RPZAtom.h"

#include "src/shared/models/base/RPZMap.hpp"

#include "base/JSONDatabase.h"
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

        const RPZMap<RPZAtom>& atoms() const;
        const QSet<RPZAssetHash>& usedAssetsIds() const;

    protected:
        void saveIntoFile();
        const int apiVersion() override;

    private:
        RPZMap<RPZAtom> _atomsById;
        QSet<RPZAssetHash> _assetHashes;

        QHash<JSONDatabase::Version, JSONDatabase::UpdateHandler> _getUpdateHandlers() override;
        JSONDatabase::Model _getDatabaseModel() override;
        void _setupLocalData() override;

};