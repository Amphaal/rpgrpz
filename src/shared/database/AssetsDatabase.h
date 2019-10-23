#pragma once

#include <algorithm>

#include <QString>
#include <QVariantHash>
#include <QCryptographicHash>
#include <QMutexLocker>

#include "src/shared/models/toy/RPZAsset.hpp"
#include "src/shared/models/toy/RPZAssetImportPackage.hpp"
#include "src/shared/database/_base/JSONDatabase.h"

#include "src/helpers/_appContext.h"

typedef QString RPZFolderPath; //internal DB arborescence path (only containers)

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
        void addAsset(const RPZAsset &asset, const RPZFolderPath &internalPathToAddTo);
        void createFolder(const RPZFolderPath &parentPath);

        bool renameFolder(const QString &newName, const RPZFolderPath &folderToRename);
        bool renameAsset(const QString &newName, const RPZAssetHash &hash);

        bool removeAssets(const QList<RPZAssetHash> &hashesToRemove);
        bool removeFolders(const QList<RPZFolderPath> &pathsToRemove);

        bool moveItemsTo(const RPZFolderPath &internalPathToMoveTo, const QList<RPZFolderPath> &topmostPathsToMove, const QList<RPZAssetHash> &topmostHashesToMove);
        
        //
        const RPZAsset asset(const RPZAssetHash &hash) const;
        const QSet<RPZAssetHash> getStoredAssetsIds() const;

        //network import/export
        void importAsset(RPZAssetImportPackage &package);
        const RPZAssetImportPackage prepareAssetPackage(const RPZAssetHash &id) const;

    signals:
        void assetRenamed(const RPZAssetHash &id, const QString &newName);

    protected:
        const int apiVersion() override;
        void _removeDatabaseLinkedFiles() override;

        QMap<RPZFolderPath, QSet<RPZAssetHash>> _paths;
        QHash<RPZAssetHash, RPZAsset> _assets;

        const RPZAsset* _asset(const RPZAssetHash &hash) const; 
        const QString _path(const StorageContainer &targetContainer) const;

        void _saveIntoFile();
    
    private:

        //singleton
        AssetsDatabase();
        AssetsDatabase(const QJsonObject &doc);
        static inline AssetsDatabase* _singleton = nullptr;

        //updates handlers
        QHash<JSONDatabase::Version, JSONDatabase::UpdateHandler> _getUpdateHandlers() override;
        JSONDatabase::Model _getDatabaseModel() override;
        void _setupLocalData() override;

        //helpers
        QString _generateNonExistingPath(const RPZFolderPath &parentPath, const QString &prefix);

};