#pragma once

#include <QHash>
#include <QString>
#include <QUrl>
#include <QDebug>
#include <QFileInfo>
#include <QFile>
#include <QUuid>
#include <QCryptographicHash>
#include <QDir>
#include <QDateTime>
#include <QRandomGenerator>

#include "JSONDatabase.h"
#include "AssetsDatabaseElement.h"

#include "src/helpers/_const.hpp"

class AssetsDatabase : public JSONDatabase, public AssetsDatabaseElement {
    public:
        AssetsDatabase();

        //CRUD methods
        bool insertAsset(QUrl &url, AssetsDatabaseElement* parent);
        bool removeItems(QList<AssetsDatabaseElement*> elemsToRemove);
        bool moveItems(QList<AssetsDatabaseElement*> targetedItems, AssetsDatabaseElement* target);
        bool createFolder(AssetsDatabaseElement* parent);
        bool rename(QString name, AssetsDatabaseElement* target);

    protected:
        const QString defaultJsonDoc() override;
        const QString dbPath() override;
    
    private:

        //helper for item substraction
        static QJsonArray _diff(QJsonArray &target, QSet<QString> &toRemoveFromTarget);

        ///prepare for alteration (delete, move) operations
        QSet<QString> _temp_pathsToAlter;
        QHash<QString, QSet<QString>> _temp_idsToAlterByPath;
        QJsonObject _temp_db_paths;
        void _prepareForAlteration(QList<AssetsDatabaseElement*> elemsToAlter);

        //static elements, descendant of root
        QHash<AssetsDatabaseElement::Type, AssetsDatabaseElement*> _staticElements;     
        void _includeStaticComponents();
        void _includeDbComponents();
                
        //helper, returns last container element created/found
        AssetsDatabaseElement* _helperPathCreation(AssetsDatabaseElement* parent, QList<QString> paths);
};