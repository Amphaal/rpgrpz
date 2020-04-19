// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

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

        using Model = QHash<QPair<QString, JSONDatabase::EntityType>, void*>;
        using UpdateHandler = std::function<void(QJsonObject&)>;
        using Version = int;

        //remove from the array the elements in the set
        static QJsonArray diff(QJsonArray &target, QSet<QString> &toRemoveFromTarget);

        JSONDatabase(const QString &logId);
        const QString dbFilePath() const;
        void changeSourceFile(const QString &newSource);
        
        void save(); //update the physical file
        static void saveAsFile(const QJsonObject &db, const QString &filepath);
        static void saveAsFile(const QJsonObject &db, QFile &fileHandler);

 protected:
        static void updateFrom(QJsonObject &base, const QString &entityKey, const QVariantMap &entity);
        static void updateFrom(QJsonObject &base, const QString &entityKey, const QSet<QString> &entity);
        static void updateFrom(QJsonObject &base, const QString &entityKey, const QVariantHash &entity);
        static void updateFrom(QJsonObject &base, const QString &entityKey, const QVariantList &entity);

        const QJsonObject& db();
        QJsonObject entityAsObject(const QString &entityKey);
        QJsonArray entityAsArray(const QString &entityKey);

        virtual const QJsonObject _updatedInnerDb() = 0;

        virtual void _removeDatabaseLinkedFiles();

        //create new file formated for new API expected version if possible
        virtual QHash<JSONDatabase::Version, JSONDatabase::UpdateHandler> _getUpdateHandlers();
        bool _handleVersionMissmatch(QJsonObject &databaseToUpdate, JSONDatabase::Version databaseToUpdateVersion);

        //pure, replace
        virtual void _setupLocalData() = 0;
        virtual JSONDatabase::Model _getDatabaseModel() = 0;
        virtual JSONDatabase::Version apiVersion() const = 0;
        JSONDatabase::Version dbVersion();
        
        /*to call from inheritors*/
        void _initDatabaseFromJSONFile(const QString &dbFilePath);
        void _setupFromDbCopy(const QJsonObject &copy);

        void log(const QString &msg);

 private:
        QString _logId;
        QJsonObject _dbCopy;
        QFile* _destfile = nullptr;

        QJsonObject _emptyDbFile();

        QJsonDocument _readAsDocument();
        void _duplicateDbFile(QString destSuffix);
        void _updateDbFile(const QJsonObject &updatedFullDatabase);

        const QString _defaultEmptyDoc();
        JSONDatabase::Version _getDbVersion(const QJsonObject &db);


        
};
inline uint qHash(const JSONDatabase::EntityType &key, uint seed = 0) {return uint(key) ^ seed;}
