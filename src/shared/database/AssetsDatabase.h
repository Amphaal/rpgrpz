#pragma once

#include <algorithm>

#include <QString>
#include <QVariantHash>
#include <QCryptographicHash>
#include <QMutexLocker>

#include "src/shared/models/RPZToyMetadata.h"
#include "base/JSONDatabase.h"

#include "src/helpers/_appContext.h"


typedef QString RPZAssetPath; //internal DB arborescence path (only containers)
typedef QVariantHash RPZAssetImportPackage;

struct SizeAndCenter { QPointF center; QSize size; };

class RPZToyMetadata;

class AssetsDatabase : public JSONDatabase {
    
    Q_OBJECT

    public:
        enum class StorageContainer {
            NPC = 301,
            FloorBrush = 401,
            Object = 501,
            Downloaded = 601,
            Background = 701
        };

        //singleton
        static AssetsDatabase* get();

        //CRUD methods
        bool createFolder(AssetsTreeViewItem* parent);
        bool insertAsset(const QUrl &url, AssetsTreeViewItem* parent);
        bool rename(QString &name, AssetsTreeViewItem* target);
        bool removeItems(const QList<AssetsTreeViewItem*> elemsToRemove);
        bool moveItemsToContainer(const QList<AssetsTreeViewItem*> selectedItemsToMove, AssetsTreeViewItem* target);

        //
        RPZToyMetadata getAssetMetadata(const RPZAssetHash &id);
        const QSet<RPZAssetHash> getStoredAssetsIds() const;

        //network import/export
        RPZToyMetadata importAsset(const RPZAssetImportPackage &package);
        RPZAssetImportPackage prepareAssetPackage(const RPZAssetHash &id);
        
        //read
        QJsonObject paths();
        QJsonObject assets();
        static QJsonObject assets(QJsonDocument &doc);

        //fpa
        QString getFilePathToAsset(AssetsTreeViewItem* asset);
        QString getFilePathToAsset(const RPZAssetHash &id);
        static QString getFilePathToAsset(const RPZAssetHash &id, const QString &ext);

    signals:
        void assetRenamed(const RPZAssetHash &id, const QString &newName);

    protected:
        const int apiVersion() override;
        void _removeDatabaseLinkedFiles() override;
    
    private:
        //
        mutable QMutex _m_withAssetsElems;
        QHash<RPZAssetHash, AssetsTreeViewItem*> _withAssetsElems;
        void _trackAssetByElem(const RPZAssetHash &assetId, AssetsTreeViewItem* elem);

        //updates handlers
        QHash<JSONDatabase::Version, JSONDatabase::UpdateHandler> _getUpdateHandlers() override;
        JSONDatabase::Model _getDatabaseModel() override;
        void _setupLocalData() override;
        
        //helpers
        static SizeAndCenter _defineSizeAndCenterToDbAsset(const QString &assetFilePath, QJsonObject &toUpdate);

        //singleton
        AssetsDatabase();
        AssetsDatabase(const QJsonObject &doc);
        
        static inline AssetsDatabase* _singleton = nullptr;

        //createFolder() helpers
        QString _generateNonExistingPath(AssetsTreeViewItem* parent, const QString &prefix);

        //rename() helpers
        void _renameItem(const QString &name, AssetsTreeViewItem* target);
        void _renameFolder(const QString &name, AssetsTreeViewItem* target);

        //insertAsset() helpers
        RPZAssetHash _getFileSignatureFromFileUri(const QUrl &url); //return the hash
        bool _moveFileToDbFolder(const QUrl &url, const RPZAssetHash &id);
        QUrl _moveFileToDbFolder(const QByteArray &data, const QString &fileExt, const RPZAssetHash &id);
        RPZToyMetadata _addAssetToDb(const RPZAssetHash &id, const QUrl &url, AssetsTreeViewItem* parent, const QString &forcedName = QString()); //returns asset metadata

        //removeItems() helpers
        QSet<RPZAssetPath> _getPathsToAlterFromList(const QList<AssetsTreeViewItem*> &elemsToAlter);
        QHash<RPZAssetPath, QSet<RPZAssetHash>> _getAssetsToAlterFromList(const QList<AssetsTreeViewItem*> &elemsToAlter);
        QSet<RPZAssetPath> _augmentPathsSetWithMissingDescendents(QSet<RPZAssetPath> &setToAugment);
        void _augmentAssetsHashWithMissingDescendents(QHash<RPZAssetPath, QSet<RPZAssetHash>> &hashToAugment, const QSet<RPZAssetPath> &morePathsToDelete);
        QList<RPZAssetHash> _removeIdsFromPaths(QJsonObject &db_paths, const QHash<RPZAssetPath, QSet<RPZAssetHash>> &idsToRemoveByPath); //returns removed ids
        void _removeAssetsFromDb(QJsonObject &db_assets, const QList<RPZAssetHash> &assetIdsToRemove);
        void _removeAssetFile(const RPZAssetHash &id, const QJsonObject &asset);

        ////////////////////////////////////
        // INITIAL Tree Injection helpers //
        ////////////////////////////////////

        QHash<AssetsTreeViewItem::Type, AssetsTreeViewItem*> _staticElements;     
        void _injectStaticStructure();
        
        void _injectDbStructure();              
            
            //returns last elem by path created
            QHash<RPZAssetPath, AssetsTreeViewItem*> _generateFolderTreeFromDb();

            //iterate through paths chunks and create missing folders at each pass, returns last folder found/created
            AssetsTreeViewItem* _recursiveElementCreator(AssetsTreeViewItem* parent, QList<QString> &pathChunks); 

            //from definitive paths, fetch items from db and generate elements
            void _generateItemsFromDb(const QHash<RPZAssetPath, AssetsTreeViewItem*> &pathsToFillWithItems);
};