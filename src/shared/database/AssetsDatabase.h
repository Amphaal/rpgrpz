#pragma once

#include <algorithm>

#include <QString>
#include <QVariantHash>
#include <QCryptographicHash>
#include <QMutexLocker>

#include "src/shared/database/_base/JSONDatabase.h"

#include "src/shared/models/toy/RPZAsset.hpp"
#include "src/shared/models/toy/RPZAssetImportPackage.hpp"

typedef QString RPZFolderPath; //internal DB arborescence path (only containers)

class AssetsDatabase : public QObject, public JSONDatabase {
    
    Q_OBJECT

    public:
        enum class StorageContainer {
            NPC = 301,
            FloorBrush = 401,
            Object = 501,
            Downloaded = 601,
            Background = 701
        };
        Q_ENUM(StorageContainer)

        //singleton
        static AssetsDatabase* get();

        //CRUD methods
        void addAsset(const RPZAsset &asset, const RPZFolderPath &internalPathToAddTo);
        void createFolder(const RPZFolderPath &parentPath);

        bool renameFolder(const QString &requestedNewFolderName, const RPZFolderPath &pathToRename);
        void renameAsset(const QString &newName, const RPZAssetHash &hash);

        void removeAssets(const QList<RPZAssetHash> &hashesToRemove);
        void removeFolders(const QList<RPZFolderPath> &pathsToRemove);

        void moveAssetsTo(const RPZFolderPath &internalPathToMoveTo, const QList<RPZAssetHash> &hashesToMove);
        void moveFoldersTo(const RPZFolderPath &internalPathToMoveTo, const QList<RPZFolderPath> &topmostPathsToMove);
        
        //
        const RPZAsset* asset(const RPZAssetHash &hash);
        const QSet<RPZAssetHash> getStoredAssetsIds() const;

        //network import/export
        bool importAsset(RPZAssetImportPackage &package);
        const RPZAssetImportPackage prepareAssetPackage(const RPZAssetHash &hash) const;

    signals:
        void assetRenamed(const RPZAssetHash &id, const QString &newName);

    protected:
        const JSONDatabase::Version apiVersion() override;
        void _removeDatabaseLinkedFiles() override;
    
        QMap<RPZFolderPath, QSet<RPZAssetHash>> _paths;
        QHash<RPZAssetHash, RPZAsset> _assets;
        QHash<RPZAssetHash, RPZFolderPath> _w_assetToPath;

        RPZAsset* _asset(const RPZAssetHash &hash); 
        const QString _path(const StorageContainer &targetContainer) const;
        static const QString _parentPath(const RPZFolderPath &toExtractParentFrom);
        static const QString _folderName(const RPZFolderPath &toExtractNameFrom);

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
        void _removeAssetFiles(const QList<RPZAsset> &toRemoveFromStorage);

        //
        typedef QHash<RPZFolderPath, QSet<RPZAssetHash>> HashesByPathToRemove;
        typedef QList<RPZAsset> RemovedAssets;

        void _removeHashesFromPaths(const HashesByPathToRemove &hashesToRemoveFromPaths);
        QPair<HashesByPathToRemove, RemovedAssets> _removeAssets(const QList<RPZAssetHash> &hashesToRemove, bool onlyRemoveReference = false);

        //
        typedef QHash<RPZFolderPath, QSet<RPZFolderPath>> StartingWithPathRequestResults;
        QSet<RPZFolderPath> _getPathsStartingWith(const RPZFolderPath &toRequest);
        StartingWithPathRequestResults _getPathsStartingWith(const QList<RPZFolderPath> &toRequest);

        void _reroutePaths(const RPZFolderPath &ancestor, const RPZFolderPath &toReplaceAncestor, const QSet<RPZFolderPath> &subjects);
};