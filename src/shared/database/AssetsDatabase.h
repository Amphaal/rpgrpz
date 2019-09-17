#pragma once

#include <algorithm>

#include <QString>
#include <QVariantHash>
#include <QCryptographicHash>

#include "src/shared/models/RPZToyMetadata.h"
#include "base/JSONDatabase.h"
#include "src/shared/assets/AssetsDatabaseElement.h"
#include "src/helpers/_appContext.h"


typedef QString RPZAssetPath; //internal DB arborescence path (only containers)
typedef QVariantHash RPZAssetImportPackage;

struct SizeAndCenter { QPointF center; QSize size; };

class RPZToyMetadata;

class AssetsDatabase : public QObject, public JSONDatabase, public AssetsDatabaseElement {
    
    Q_OBJECT

    public:
        //singleton
        static AssetsDatabase* get();

        //CRUD methods
        bool createFolder(AssetsDatabaseElement* parent);
        bool insertAsset(const QUrl &url, AssetsDatabaseElement* parent);
        bool rename(QString &name, AssetsDatabaseElement* target);
        bool removeItems(const QList<AssetsDatabaseElement*> elemsToRemove);
        bool moveItems(const QList<AssetsDatabaseElement*> selectedItemsToMove, AssetsDatabaseElement* target);

        //
        RPZToyMetadata getAssetMetadata(const RPZAssetHash &id);

        //network import/export
        RPZToyMetadata importAsset(const RPZAssetImportPackage &package);
        RPZAssetImportPackage prepareAssetPackage(const RPZAssetHash &id);
        
        //read
        QJsonObject paths();
        QJsonObject assets();
        static QJsonObject assets(QJsonDocument &doc);

        //fpa
        QString getFilePathToAsset(AssetsDatabaseElement* asset);
        QString getFilePathToAsset(const RPZAssetHash &id);
        static QString getFilePathToAsset(const RPZAssetHash &id, const QString &ext);

    signals:
        void assetRenamed(const RPZAssetHash &id, const QString &newName);

    protected:
        const QString defaultJsonDoc() override;
        const QString dbPath() override;
        const int apiVersion() override;

        void _removeDatabaseLinkedFiles() override;
    
    private:
        //
        QHash<RPZAssetHash, AssetsDatabaseElement*> _withAssetsElems;

        //updates handlers
        QHash<JSONDatabaseVersion, JSONDatabaseUpdateHandler> _getUpdateHandlers() override;
        
        //helpers
        static SizeAndCenter _defineSizeAndCenterToDbAsset(const QString &assetFilePath, QJsonObject &toUpdate);

        //singleton
        AssetsDatabase();
        static inline AssetsDatabase* _singleton = nullptr;

        //createFolder() helpers
        QString _generateNonExistingPath(AssetsDatabaseElement* parent, const QString &prefix);

        //rename() helpers
        void _renameItem(const QString &name, AssetsDatabaseElement* target);
        void _renameFolder(const QString &name, AssetsDatabaseElement* target);

        //insertAsset() helpers
        RPZAssetHash _getFileSignatureFromFileUri(const QUrl &url); //return the hash
        bool _moveFileToDbFolder(const QUrl &url, const RPZAssetHash &id);
        QUrl _moveFileToDbFolder(const QByteArray &data, const QString &fileExt, const RPZAssetHash &id);
        RPZToyMetadata _addAssetToDb(const RPZAssetHash &id, const QUrl &url, AssetsDatabaseElement* parent, const QString &forcedName = QString()); //returns asset metadata

        //removeItems() helpers
        QSet<RPZAssetPath> _getPathsToAlterFromList(const QList<AssetsDatabaseElement*> &elemsToAlter);
        QHash<RPZAssetPath, QSet<RPZAssetHash>> _getAssetsToAlterFromList(const QList<AssetsDatabaseElement*> &elemsToAlter);
        QSet<RPZAssetPath> _augmentPathsSetWithMissingDescendents(QSet<RPZAssetPath> &setToAugment);
        void _augmentAssetsHashWithMissingDescendents(QHash<RPZAssetPath, QSet<RPZAssetHash>> &hashToAugment, const QSet<RPZAssetPath> &morePathsToDelete);
        QList<RPZAssetHash> _removeIdsFromPaths(QJsonObject &db_paths, const QHash<RPZAssetPath, QSet<RPZAssetHash>> &idsToRemoveByPath); //returns removed ids
        void _removeAssetsFromDb(QJsonObject &db_assets, const QList<RPZAssetHash> &assetIdsToRemove);
        void _removeAssetFile(const RPZAssetHash &id, const QJsonObject &asset);

        ////////////////////////////////////
        // INITIAL Tree Injection helpers //
        ////////////////////////////////////

        QHash<AssetsDatabaseElement::Type, AssetsDatabaseElement*> _staticElements;     
        void _injectStaticStructure();
        
        void _injectDbStructure();              
            
            //returns last elem by path created
            QHash<RPZAssetPath, AssetsDatabaseElement*> _generateFolderTreeFromDb();

            //iterate through paths chunks and create missing folders at each pass, returns last folder found/created
            AssetsDatabaseElement* _recursiveElementCreator(AssetsDatabaseElement* parent, QList<QString> &pathChunks); 

            //from definitive paths, fetch items from db and generate elements
            void _generateItemsFromDb(const QHash<RPZAssetPath, AssetsDatabaseElement*> &pathsToFillWithItems);
};