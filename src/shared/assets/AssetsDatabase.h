#pragma once

#include <algorithm>

#include <QHash>
#include <QString>
#include <QUrl>
#include <QDebug>
#include <QFileInfo>
#include <QFile>
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
        bool createFolder(AssetsDatabaseElement* parent);
        bool insertAsset(QUrl &url, AssetsDatabaseElement* parent);
        bool rename(QString name, AssetsDatabaseElement* target);
        bool removeItems(QList<AssetsDatabaseElement*> elemsToRemove);
        bool moveItems(QList<AssetsDatabaseElement*> selectedItemsToMove, AssetsDatabaseElement* target);

        //read
        QJsonObject paths();
        QJsonObject assets();
        QJsonArray hashes(); 

    protected:
        const QString defaultJsonDoc() override;
        const QString dbPath() override;
    
    private:

        //createFolder() helpers
        QString _generateNonExistingPath(AssetsDatabaseElement* parent, QString prefix);

        //rename() helpers
        void _renameItem(QString &name, AssetsDatabaseElement* target);
        void _renameFolder(QString &name, AssetsDatabaseElement* target);

        //insertAsset() helpers
        QString _getHashFromFileUri(QUrl &url); //return the hash
        QString _moveFileToDbFolder(QUrl &url); //returns an unique id
        QString _addAssetToDb(QString &id, QUrl &url, QString &fileHash, AssetsDatabaseElement* parent); //returns a default displayname

        //removeItems() helpers
        QSet<QString> _getPathsToAlterFromList(QList<AssetsDatabaseElement*> &elemsToAlter);
        QHash<QString, QSet<QString>> _getAssetsToAlterFromList(QList<AssetsDatabaseElement*> &elemsToAlter);
        QSet<QString> _augmentPathsSetWithMissingDescendents(QSet<QString> &setToAugment);
        void _augmentAssetsHashWithMissingDescendents(QHash<QString, QSet<QString>> &hashToAugment, QSet<QString> &morePathsToDelete);
        QList<QString> _removeIdsFromPaths(QJsonObject &db_paths, QHash<QString, QSet<QString>> &idsToRemoveByPath); //returns removed ids
        QSet<QString> _removeAssetsFromDb(QJsonObject &db_assets, QList<QString> &assetIdsToRemove); //returns hashes to remove
        void _removeAssetFile(QString &id, QJsonObject &asset);

        ////////////////////////////////////
        // INITIAL Tree Injection helpers //
        ////////////////////////////////////

        QHash<AssetsDatabaseElement::Type, AssetsDatabaseElement*> _staticElements;     
        void _injectStaticStructure();
        
        void _injectDbStructure();              
            
            //returns last elem by path created
            QHash<QString, AssetsDatabaseElement*> _generateFolderTreeFromDb();

            //iterate through paths chunks and create missing folders at each pass, returns last folder found/created
            AssetsDatabaseElement* _recursiveElementCreator(AssetsDatabaseElement* parent, QList<QString> pathChunks); 

            //from definitive paths, fetch items from db and generate elements
            void _generateItemsFromDb(QHash<QString, AssetsDatabaseElement*> &pathsToFillWithItems);


};