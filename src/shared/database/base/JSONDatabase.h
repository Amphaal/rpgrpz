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

enum JSONDatabaseEntityType {
    ET_Object,
    ET_Array
};

typedef int JSONDatabaseVersion;
typedef std::function<void(QJsonObject&)> JSONDatabaseUpdateHandler;
typedef QHash<QPair<QString, JSONDatabaseEntityType>, void*> JSONDatabaseModel;

class JSONDatabase {

    public:
        //remove from the array the elements in the set
        static QJsonArray diff(QJsonArray &target, QSet<QString> &toRemoveFromTarget);

    protected:
        JSONDatabase(const QString &dbFilePath);
        JSONDatabase(const QJsonObject &obj);

        static void updateFrom(QJsonObject &base, const QString &entityKey, const QVariantMap &entity);
        static void updateFrom(QJsonObject &base, const QString &entityKey, const QSet<QString> &entity);

        const QJsonObject& db();
        QJsonObject entityAsObject(const QString &entityKey);
        QJsonArray entityAsArray(const QString &entityKey);

        //update the physical file
        void _updateDbFile(const QJsonObject &updatedFullDatabase);

        virtual void _removeDatabaseLinkedFiles();

        //create new file formated for new API expected version if possible
        virtual QHash<JSONDatabaseVersion, JSONDatabaseUpdateHandler> _getUpdateHandlers();
        bool _handleVersionMissmatch(QJsonObject &databaseToUpdate, int databaseToUpdateVersion);

        //pure, replace
        virtual void _setupLocalData() = 0;
        virtual JSONDatabaseModel _getDatabaseModel() = 0;
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
        JSONDatabaseVersion _getDbVersion(const QJsonObject &db);
        void _setupFromDbCopy(const QJsonObject &copy);
};