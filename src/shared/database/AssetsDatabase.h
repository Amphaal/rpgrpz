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
// different license and copyright still refer to this GNU General Public License.

#pragma once

#include <algorithm>

#include <QString>
#include <QVariantHash>
#include <QCryptographicHash>
#include <QMutexLocker>

#include "src/shared/database/_base/JSONDatabase.h"

#include "src/shared/models/toy/RPZAsset.hpp"
#include "src/shared/models/toy/RPZAssetImportPackage.hpp"

class AssetsDatabase : public QObject, public JSONDatabase {
    
    Q_OBJECT

    public:
        using FolderPath = QString; //internal DB arborescence path (only containers)

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
        void addAsset(const RPZAsset &asset, const AssetsDatabase::FolderPath &internalPathToAddTo);
        const QString createFolder(const AssetsDatabase::FolderPath &parentPath);

        bool renameFolder(const QString &requestedNewFolderName, const AssetsDatabase::FolderPath &pathToRename);
        void renameAsset(const QString &newName, const RPZAsset::Hash &hash);

        void removeAssets(const QList<RPZAsset::Hash> &hashesToRemove);
        void removeFolders(const QList<AssetsDatabase::FolderPath> &topMostPathsToRemove);

        void moveAssetsTo(const AssetsDatabase::FolderPath &internalPathToMoveTo, const QList<RPZAsset::Hash> &hashesToMove);
        void moveFoldersTo(const AssetsDatabase::FolderPath &internalPathToMoveTo, const QList<AssetsDatabase::FolderPath> &topmostPathsToMove);
        
        //
        const QMap<AssetsDatabase::FolderPath, QSet<RPZAsset::Hash>> paths() const;
        const QHash<RPZAsset::Hash, RPZAsset> assets() const;

        //
        const RPZAsset* asset(const RPZAsset::Hash &hash);
        const QSet<RPZAsset::Hash> getStoredAssetHashes() const;

        //network import/export
        bool importAsset(RPZAssetImportPackage &package);
        const RPZAssetImportPackage prepareAssetPackage(const RPZAsset::Hash &hash) const;

    signals:
        void assetRenamed(const RPZAsset::Hash &id, const QString &newName);

    protected:
        JSONDatabase::Version apiVersion() const override;
        void _removeDatabaseLinkedFiles() override;
    
        QMap<AssetsDatabase::FolderPath, QSet<RPZAsset::Hash>> _paths;
        QHash<RPZAsset::Hash, RPZAsset> _assets;
        QHash<RPZAsset::Hash, AssetsDatabase::FolderPath> _w_assetToPath;

        RPZAsset* _asset(const RPZAsset::Hash &hash); 
        const QString _path(const StorageContainer &targetContainer) const;
        static const QString _parentPath(const AssetsDatabase::FolderPath &toExtractParentFrom);
        static const QString _folderName(const AssetsDatabase::FolderPath &toExtractNameFrom);

        const QJsonObject _updatedInnerDb() override;
    
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
        QString _generateNonExistingPath(const AssetsDatabase::FolderPath &parentPath, const QString &prefix);
        void _removeAssetFiles(const QList<RPZAsset> &toRemoveFromStorage);

        //
        using HashesByPathToRemove = QHash<AssetsDatabase::FolderPath, QSet<RPZAsset::Hash>>;
        using RemovedAssets = QList<RPZAsset>;

        void _removeHashesFromPaths(const HashesByPathToRemove &hashesToRemoveFromPaths);
        QPair<HashesByPathToRemove, RemovedAssets> _removeAssets(const QList<RPZAsset::Hash> &hashesToRemove, bool onlyRemoveReference = false);

        //
        using StartingWithPathRequestResults = QHash<AssetsDatabase::FolderPath, QSet<AssetsDatabase::FolderPath>>;
        QSet<AssetsDatabase::FolderPath> _getPathsStartingWith(const AssetsDatabase::FolderPath &toRequest);
        StartingWithPathRequestResults _getPathsStartingWith(const QList<AssetsDatabase::FolderPath> &toRequest);

        void _reroutePaths(const AssetsDatabase::FolderPath &ancestor, const AssetsDatabase::FolderPath &toReplaceAncestor, const QSet<AssetsDatabase::FolderPath> &subjects);
};