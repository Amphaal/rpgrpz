#pragma once

#include <QString>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QSet>
#include <QUuid>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

class JSONDatabase {
    public:
        JSONDatabase();

        //remove from the array the elements in the set
        static QJsonArray diff(QJsonArray &target, QSet<QString> &toRemoveFromTarget);

        static QString generateId();

    protected:
        QJsonDocument _db;
        QFile* _destfile = nullptr;

        //must be called from inherited constructor
        void _instanciateDb();

        //update the physical file
        void _updateDbFile(QJsonObject &newData);

        //recreate file if doesnt exist
        void _checkFileExistance();

        //pure, replace
        virtual const QString defaultJsonDoc() = 0;
        virtual const QString dbPath() = 0;
        
};