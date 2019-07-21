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

typedef int JSONDatabaseVersion;
typedef std::function<void(QJsonDocument&)> JSONDatabaseUpdateHandler;

class JSONDatabase {

    public:
        JSONDatabase();

        //remove from the array the elements in the set
        static QJsonArray diff(QJsonArray &target, QSet<QString> &toRemoveFromTarget);

        static JSONDatabaseVersion getDbVersion(QJsonDocument &db); 

    protected:
        QJsonDocument _db;
        QFile* _destfile = nullptr;

        //must be called from inherited constructor
        void _instanciateDb();

        //update the physical file
        void _updateDbFile(QJsonObject &newData);

        //recreate file if doesnt exist
        void _checkFileExistance();

        virtual void _removeDatabaseLinkedFiles();

        //create new file formated for new API expected version if possible
        virtual QHash<JSONDatabaseVersion, JSONDatabaseUpdateHandler> _getUpdateHandlers();
        bool _handleVersionMissmatch(QJsonDocument &databaseToUpdate, int databaseToUpdateVersion);

        //pure, replace
        virtual const QString defaultJsonDoc() = 0;
        virtual const QString dbPath() = 0;
        virtual const int apiVersion() = 0;
        const int dbVersion();

    private:
        void _createEmptyDbFile();
        QJsonDocument _readDbFile();
        void _duplicateDbFile(QString destSuffix);
};