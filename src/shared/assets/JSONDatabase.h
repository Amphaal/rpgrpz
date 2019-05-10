#pragma once

#include <QString>
#include <QFile>
#include <QDir>
#include <QDebug>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

class JSONDatabase {
    public:
        JSONDatabase();

    protected:
        QJsonDocument _db;
        QFile* _destfile = nullptr;

        void _updateDbFile(QJsonObject &newData);
        void _checkFileExistance();

        //pure
        virtual const QString defaultJsonDoc() = 0;
        virtual const QString dbPath() = 0;
        void _instanciateDb();
};