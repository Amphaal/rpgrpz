#pragma once

#include <QString>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QSet>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

class JSONDatabase {
    public:
        JSONDatabase();

        //remove from the array the elements in the set
        static QJsonArray diff(QJsonArray &target, QSet<QString> &toRemoveFromTarget);

    protected:
        QJsonDocument _db;
        QFile* _destfile = nullptr;

        //must be called from inherited constructor
        void _instanciateDb();

        //update the physical file
        void _updateDbFile(QJsonObject &newData);

        //recreate file if doesnt exist
        void _checkFileExistance();

        virtual void _removeDatabase();

        //pure, replace
        virtual const QString defaultJsonDoc() = 0;
        virtual const QString dbPath() = 0;
        virtual const int apiVersion() = 0;
        virtual const int dbVersion() = 0;
        
};