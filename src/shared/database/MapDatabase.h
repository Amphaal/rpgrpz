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

        void saveIntoFile(const RPZMap<RPZAtom> &atoms);
        static QJsonObject toObject(const RPZMap<RPZAtom> &atoms, const QJsonDocument &doc);

        RPZMap<RPZAtom> toAtoms();
        static RPZMap<RPZAtom> toAtoms(const QJsonDocument &doc);

        const QSet<RPZAssetHash> getUsedAssetsIds() const;

    protected:
        const QString defaultJsonDoc() override;
        const QString dbPath() override;
        const int apiVersion() override;

    private:
        QString _filePath;

        QHash<JSONDatabaseVersion, JSONDatabaseUpdateHandler> _getUpdateHandlers() override;
};