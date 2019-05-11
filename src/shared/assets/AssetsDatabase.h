#pragma once

#include <QHash>
#include <QString>
#include <QUrl>
#include <QDebug>
#include <QFileInfo>
#include <QUuid>
#include <QCryptographicHash>
#include <QDir>
#include <QDateTime>

#include "JSONDatabase.h"
#include "AssetsDatabaseElement.h"

#include "src/helpers/_const.hpp"

class AssetsDatabase : public JSONDatabase, public AssetsDatabaseElement {
    public:
        AssetsDatabase();

        //CRUD methods
        bool insertAsset(QUrl &url, AssetsDatabaseElement* parent);
        bool removeItems(QList<AssetsDatabaseElement*> itemsToRemove);
        bool moveItems(QList<AssetsDatabaseElement*> targetedItems, AssetsDatabaseElement* target);
        bool createFolder(AssetsDatabaseElement* parent);
        bool rename(QString name, AssetsDatabaseElement* target);

    protected:
        const QString defaultJsonDoc() override;
        const QString dbPath() override;
    
    private:
        void _includeStaticComponents();
        void _includeDbComponents();

        //helper, returns last container element created/found
        AssetsDatabaseElement* _helperPathCreation(AssetsDatabaseElement* parent, QList<QString> paths);

        QHash<AssetsDatabaseElement::Type, AssetsDatabaseElement*> _staticElements;
};