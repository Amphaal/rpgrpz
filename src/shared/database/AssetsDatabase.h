#pragma once

#include <algorithm>

#include <QString>
#include <QVariantHash>
#include <QCryptographicHash>

#include "src/shared/models/RPZAssetMetadata.h"
#include "base/JSONDatabase.h"
#include "src/shared/assets/AssetsDatabaseElement.h"
#include "src/helpers/_appContext.h"


typedef QString RPZAssetPath; //internal DB arborescence path (only containers)
typedef QVariantHash RPZAssetImportPackage;

class RPZAssetMetadata;

class AssetsDatabase : public QObject, public JSONDatabase, public AssetsDatabaseElement {
    
    Q_OBJECT

    public:
        //singleton
        static AssetsDatabase* get();

        //CRUD methods
        bool createFolder(AssetsDatabaseElement* parent);
        bool insertAsset(QUrl &url, AssetsDatabaseElement* parent);
        bool rename(QString name, AssetsDatabaseElement* target);
        bool removeItems(QList<AssetsDatabaseElement*> elemsToRemove);
        bool moveItems(QList<AssetsDatabaseElement*> selectedItemsToMove, AssetsDatabaseElement* target);

        //network import/export
        RPZAssetMetadata importAsset(const RPZAssetImportPackage &package);
        RPZAssetImportPackage prepareAssetPackage(const RPZAssetHash &id);
        
        //read
        QJsonObject paths();
        QJsonObject assets();

        QString getFilePathToAsset(AssetsDatabaseElement* asset);
        QString getFilePathToAsset(const RPZAssetHash &id);

        static QString assetsStorageFilepath();

    signals:
        void assetRenamed(const RPZAssetHash &id, const QString &newName);

    protected:
        const QString defaultJsonDoc() override;
        const QString dbPath() override;
        const int apiVersion() override;

        void _removeDatabaseLinkedFiles() override;
    
    private:
        //singleton
        AssetsDatabase();
        static inline AssetsDatabase* _singleton = nullptr;

        //createFolder() helpers
        QString _generateNonExistingPath(AssetsDatabaseElement* parent, QString prefix);

        //rename() helpers
        void _renameItem(QString &name, AssetsDatabaseElement* target);
        void _renameFolder(QString &name, AssetsDatabaseElement* target);

        //insertAsset() helpers
        RPZAssetHash _getFileSignatureFromFileUri(QUrl &url); //return the hash
        bool _moveFileToDbFolder(QUrl &url, RPZAssetHash &id);
        QUrl _moveFileToDbFolder(QByteArray &data, QString &fileExt, QString &name, RPZAssetHash &id);
        QString _addAssetToDb(RPZAssetHash &id, QUrl &url, AssetsDatabaseElement* parent); //returns a default displayname

        //removeItems() helpers
        QSet<RPZAssetPath> _getPathsToAlterFromList(QList<AssetsDatabaseElement*> &elemsToAlter);
        QHash<RPZAssetPath, QSet<RPZAssetHash>> _getAssetsToAlterFromList(QList<AssetsDatabaseElement*> &elemsToAlter);
        QSet<RPZAssetPath> _augmentPathsSetWithMissingDescendents(QSet<RPZAssetPath> &setToAugment);
        void _augmentAssetsHashWithMissingDescendents(QHash<RPZAssetPath, QSet<RPZAssetHash>> &hashToAugment, QSet<RPZAssetPath> &morePathsToDelete);
        QList<RPZAssetHash> _removeIdsFromPaths(QJsonObject &db_paths, QHash<RPZAssetPath, QSet<RPZAssetHash>> &idsToRemoveByPath); //returns removed ids
        void _removeAssetsFromDb(QJsonObject &db_assets, QList<RPZAssetHash> &assetIdsToRemove);
        void _removeAssetFile(RPZAssetHash &id, QJsonObject &asset);

        ////////////////////////////////////
        // INITIAL Tree Injection helpers //
        ////////////////////////////////////

        QHash<AssetsDatabaseElement::Type, AssetsDatabaseElement*> _staticElements;     
        void _injectStaticStructure();
        
        void _injectDbStructure();              
            
            //returns last elem by path created
            QHash<RPZAssetPath, AssetsDatabaseElement*> _generateFolderTreeFromDb();

            //iterate through paths chunks and create missing folders at each pass, returns last folder found/created
            AssetsDatabaseElement* _recursiveElementCreator(AssetsDatabaseElement* parent, QList<QString> pathChunks); 

            //from definitive paths, fetch items from db and generate elements
            void _generateItemsFromDb(QHash<RPZAssetPath, AssetsDatabaseElement*> &pathsToFillWithItems);
};