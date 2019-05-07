#pragma once

#include <QString>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QDebug>

class JSONDatabase {
    public:
        JSONDatabase();

    protected:
        QJsonDocument* _db = nullptr;

        //pure
        virtual const QString defaultJsonDoc() = 0;
        virtual const QString dbPath() = 0;
        void _instanciateDb();
};