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
#include "src/shared/models/_base/RPZMap.hpp"

class JSONDatabase {

    public:
        enum class EntityType {
            Object,
            Array
        };

        typedef QHash<QPair<QString, JSONDatabase::EntityType>, void*> Model;
        typedef std::function<void(QJsonObject&)> UpdateHandler;
        typedef int Version;

        //remove from the array the elements in the set
        static QJsonArray diff(QJsonArray &target, QSet<QString> &toRemoveFromTarget);

        JSONDatabase();
        const QString dbFilePath();

    protected:

        static void updateFrom(QJsonObject &base, const QString &entityKey, const QVariantMap &entity);
        static void updateFrom(QJsonObject &base, const QString &entityKey, const QSet<QString> &entity);
        static void updateFrom(QJsonObject &base, const QString &entityKey, const QVariantHash &entity);

        const QJsonObject& db();
        QJsonObject entityAsObject(const QString &entityKey);
        QJsonArray entityAsArray(const QString &entityKey);

        //update the physical file
        void _updateDbFile(const QJsonObject &updatedFullDatabase);

        virtual void _removeDatabaseLinkedFiles();

        //create new file formated for new API expected version if possible
        virtual QHash<JSONDatabase::Version, JSONDatabase::UpdateHandler> _getUpdateHandlers();
        bool _handleVersionMissmatch(QJsonObject &databaseToUpdate, JSONDatabase::Version databaseToUpdateVersion);

        //pure, replace
        virtual void _setupLocalData() = 0;
        virtual JSONDatabase::Model _getDatabaseModel() = 0;
        virtual const JSONDatabase::Version apiVersion() const = 0;
        const JSONDatabase::Version dbVersion();
        
        /*to call from inheritors*/
        void _initDatabaseFromJSONFile(const QString &dbFilePath);
        void _setupFromDbCopy(const QJsonObject &copy);

    private:
        QJsonObject _dbCopy;
        QFile* _destfile = nullptr;

        void _createEmptyDbFile();
        QJsonDocument _readAsDocument();
        void _duplicateDbFile(QString destSuffix);
        void _defineDatabaseObject(const QJsonDocument &document);

        const QString _defaultEmptyDoc();
        JSONDatabase::Version _getDbVersion(const QJsonObject &db);
        
};
inline uint qHash(const JSONDatabase::EntityType &key, uint seed = 0) {return uint(key) ^ seed;}
