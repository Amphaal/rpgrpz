#pragma once

#include <QObject>

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

#include "base/JSONDatabase.h"
#include "src/shared/assets/AssetsDatabaseElement.h"

#include "src/helpers/_appContext.h"

typedef QString RPZAssetPath; //internal DB arborescence path (only containers)

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
        QVariantHash prepareAssetPackage(RPZAssetId &id);
        RPZAssetId importAsset(const QVariantHash &package);

        //read
        QJsonObject paths();
        QJsonObject assets();

        QString getFilePathToAsset(AssetsDatabaseElement* asset);
        QString getFilePathToAsset(const RPZAssetId &id);

        static QString assetsStorageFilepath();

    signals:
        void assetRenamed(const RPZAssetId &id, const QString &newName);

    protected:
        const QString defaultJsonDoc() override;
        const QString dbPath() override;
        const int apiVersion() override;
        const int dbVersion() override;

        void _removeDatabase() override;
    
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
        RPZAssetId _getFileSignatureFromFileUri(QUrl &url); //return the hash
        bool _moveFileToDbFolder(QUrl &url, RPZAssetId &id);
        QUrl _moveFileToDbFolder(QByteArray &data, QString &fileExt, QString &name, RPZAssetId &id);
        QString _addAssetToDb(RPZAssetId &id, QUrl &url, AssetsDatabaseElement* parent); //returns a default displayname

        //removeItems() helpers
        QSet<RPZAssetPath> _getPathsToAlterFromList(QList<AssetsDatabaseElement*> &elemsToAlter);
        QHash<RPZAssetPath, QSet<RPZAssetId>> _getAssetsToAlterFromList(QList<AssetsDatabaseElement*> &elemsToAlter);
        QSet<RPZAssetPath> _augmentPathsSetWithMissingDescendents(QSet<RPZAssetPath> &setToAugment);
        void _augmentAssetsHashWithMissingDescendents(QHash<RPZAssetPath, QSet<RPZAssetId>> &hashToAugment, QSet<RPZAssetPath> &morePathsToDelete);
        QList<RPZAssetId> _removeIdsFromPaths(QJsonObject &db_paths, QHash<RPZAssetPath, QSet<RPZAssetId>> &idsToRemoveByPath); //returns removed ids
        void _removeAssetsFromDb(QJsonObject &db_assets, QList<RPZAssetId> &assetIdsToRemove);
        void _removeAssetFile(RPZAssetId &id, QJsonObject &asset);

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