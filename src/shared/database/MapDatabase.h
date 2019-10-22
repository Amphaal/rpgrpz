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

        void addAtom(const RPZAtom &toAdd);
        void updateAtom(const RPZAtom &updated);
        void removeAtom(const RPZAtomId &idToRemove);

    protected:
        void saveIntoFile();
        const int apiVersion() override;

        const RPZMap<RPZAtom>& atoms() const;
        const QSet<RPZAssetHash>& getUsedAssetsIds() const;

    private:
        RPZMap<RPZAtom> _atoms;
        QSet<RPZAssetHash> _assetHashes;

        QHash<JSONDatabaseVersion, JSONDatabaseUpdateHandler> _getUpdateHandlers() override;
        JSONDatabaseModel _getDatabaseModel() override;
        void _setupLocalData() override;

};