#pragma once

#include <algorithm>
#include <vector>

#include <QString>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QSet>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

#include "src/helpers/_appContext.h"
#include "src/shared/models/base/RPZMap.hpp"

typedef int JSONDatabaseVersion;
typedef std::function<void(QJsonObject&)> JSONDatabaseUpdateHandler;
typedef QHash<QString, void*> JSONDatabaseModel;

class JSONDatabase {

    public:
        JSONDatabase();

        //remove from the array the elements in the set
        static QJsonArray diff(QJsonArray &target, QSet<QString> &toRemoveFromTarget);

        static JSONDatabaseVersion getDbVersion(const QJsonObject &db); 

    protected:
        static void updateFrom(QJsonObject &base, const QString &entityKey, const QVariantMap &entity);

        const QJsonObject& db();
        QJsonObject entity(const QString &entityKey);

        //must be called from inherited constructor
        void _instanciateDb();

        //update the physical file
        void _updateDbFile(const QJsonObject &updatedFullDatabase);
        void _updateDbFile(const QVariantHash &updatedFullDatabase);

        //recreate file if doesnt exist
        void _checkFileExistance();

        virtual void _removeDatabaseLinkedFiles();

        //create new file formated for new API expected version if possible
        virtual QHash<JSONDatabaseVersion, JSONDatabaseUpdateHandler> _getUpdateHandlers();
        bool _handleVersionMissmatch(QJsonObject &databaseToUpdate, int databaseToUpdateVersion);

        //pure, replace
        virtual JSONDatabaseModel _getDatabaseModel() = 0;
        virtual const QString dbPath() = 0;
        virtual const int apiVersion() = 0;
        const int dbVersion();

    private:
        QJsonObject _dbCopy;
        QFile* _destfile = nullptr;

        void _createEmptyDbFile();
        QJsonDocument _readAsDocument();
        void _duplicateDbFile(QString destSuffix);
        void _defineDatabaseObject(const QJsonDocument &document);

        const QString _defaultEmptyDoc();
};