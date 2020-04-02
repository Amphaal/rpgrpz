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

#include "AssetsDatabase.h"

AssetsDatabase::AssetsDatabase(const QJsonObject &doc) : JSONDatabase(QStringLiteral(u"AssetsDB")) { 
    this->_setupFromDbCopy(doc);
}
AssetsDatabase::AssetsDatabase() : JSONDatabase(QStringLiteral(u"AssetsDB")) {
    this->_initDatabaseFromJSONFile(
        AppContext::getAssetsFileCoordinatorLocation()
    );
};

AssetsDatabase* AssetsDatabase::get() {
    if(!_singleton) {
        _singleton = new AssetsDatabase();
    }
    return _singleton;
}

JSONDatabase::Version AssetsDatabase::apiVersion() const {
    return 5;
}

JSONDatabase::Model AssetsDatabase::_getDatabaseModel() {
    return {
        { { QStringLiteral(u"paths"), JSONDatabase::EntityType::Object }, &this->_paths },
        { { QStringLiteral(u"assets"), JSONDatabase::EntityType::Object }, &this->_assets }
    };
}

const QJsonObject AssetsDatabase::_updatedInnerDb() {
    
    auto db = this->db();

    //assets
    QVariantHash assets;
    for(const auto &asset : this->_assets) {
        assets.insert(asset.hash(), asset);
    }
    updateFrom(db, QStringLiteral(u"assets"), assets);

    //paths
    QVariantMap paths;
    for(auto i = this->_paths.begin(); i != this->_paths.end(); i++) {
        
        QVariantList hashes;
        for(const auto &hash : i.value()) hashes += hash;

        paths.insert(i.key(), hashes);

    }
    updateFrom(db, QStringLiteral(u"paths"), paths);

    return db;

}

void AssetsDatabase::_setupLocalData() {

    //paths
    auto paths = this->entityAsObject(QStringLiteral(u"paths"));
    for(auto i = paths.begin(); i != paths.end(); i++) {
        
        auto path = i.key();
        auto hashes = i.value().toVariant().toStringList();
        auto hSet = QSet<QString>(hashes.begin(), hashes.end());

        this->_paths.insert(path, hSet);

    }

    //assets
    auto assets = this->entityAsObject(QStringLiteral(u"assets"));
    for(auto i = assets.begin(); i != assets.end(); i++) {
        
        auto hash = i.key();
        RPZAsset asset(i.value().toVariant().toHash(), hash);

        this->_assets.insert(hash, asset);

    }

    //wlink, internal only
    for(auto i = this->_paths.begin(); i != this->_paths.end(); i++) {
        auto path = i.key();
        for(const auto &hashes : i.value()) {
            this->_w_assetToPath.insert(hashes, i.key());
        }
    }


}

void AssetsDatabase::_removeDatabaseLinkedFiles() {
    QDir(AppContext::getAssetsFolderLocation()).removeRecursively();
}

const QSet<RPZAsset::Hash> AssetsDatabase::getStoredAssetHashes() const {
    auto keys = this->_assets.keys();
    return QSet<QString>(keys.begin(), keys.end());
}

const QMap<AssetsDatabase::FolderPath, QSet<RPZAsset::Hash>> AssetsDatabase::paths() const {
    return this->_paths;
}

const QHash<RPZAsset::Hash, RPZAsset> AssetsDatabase::assets() const {
    return this->_assets;
}

const RPZAsset* AssetsDatabase::asset(const RPZAsset::Hash &hash) {
    return this->_asset(hash);
}

RPZAsset* AssetsDatabase::_asset(const RPZAsset::Hash &hash) {
    if(!this->_assets.contains(hash)) {
        return nullptr;
    }
    return &this->_assets[hash];
}

const QString AssetsDatabase::_path(const StorageContainer &targetContainer) const {
    auto path = QStringLiteral(u"/{%1}").arg(QString::number((int)targetContainer));
    return path;
}

const RPZAssetImportPackage AssetsDatabase::prepareAssetPackage(const RPZAsset::Hash &hash) const {
    
    if(!this->_assets.contains(hash)) return RPZAssetImportPackage();

    RPZAssetImportPackage package(this->_assets.value(hash));
    return package;

}

///
///
///

void AssetsDatabase::addAsset(const RPZAsset &asset, const AssetsDatabase::FolderPath &internalPathToAddTo) {
    
    //asset
    auto hash = asset.hash();
    this->_assets.insert(hash, asset);

    //weak link
    this->_w_assetToPath.insert(hash, internalPathToAddTo);
    
    //path
    auto &assetsOfPath = this->_paths[internalPathToAddTo];
    assetsOfPath.insert(asset.hash());

    //save
    this->save();

}

bool AssetsDatabase::importAsset(RPZAssetImportPackage &package) {
    
    auto success = package.tryIntegratePackage();
    if(!success) return false;

    this->addAsset(
        package, 
        this->_path(StorageContainer::Downloaded)
    );

    return true;

}

const QString AssetsDatabase::createFolder(const AssetsDatabase::FolderPath &parentPath) {
    
    //get a unique folder
    auto uniquePath = this->_generateNonExistingPath(parentPath, QObject::tr("Folder"));

    //add
    this->_paths.insert(uniquePath, {});

    //save
    this->save();

    return this->_folderName(uniquePath);

}

bool AssetsDatabase::renameFolder(const QString &requestedNewFolderName, const AssetsDatabase::FolderPath &pathToRename) {
    
    //prevent slashes
    if(requestedNewFolderName.contains(QStringLiteral(u"/"))) return false;

    //check if path already exists
    auto toRequest = _parentPath(pathToRename) + requestedNewFolderName;
    if(this->_paths.contains(toRequest)) return false;

    //get all paths starting with renamed path
    auto oldPathsToReroute = this->_getPathsStartingWith(pathToRename);
    this->_reroutePaths(pathToRename, toRequest, oldPathsToReroute);

    //save
    this->save();
    return true;

}

void AssetsDatabase::renameAsset(const QString &newName, const RPZAsset::Hash &hash) {
    
    //find
    auto assetToRename = this->_asset(hash);
    if(!assetToRename) return;

    //apply
    assetToRename->rename(newName);

    //emit
    emit assetRenamed(
        assetToRename->hash(), 
        newName
    );

    //save
    this->save();

}

void AssetsDatabase::removeAssets(const QList<RPZAsset::Hash> &hashesToRemove) {
    
    //clear assets
    auto result = this->_removeAssets(hashesToRemove);

    //clear paths
    this->_removeHashesFromPaths(result.first);

    //remove stored files
    this->_removeAssetFiles(result.second);

    //save
    this->save();

}

void AssetsDatabase::removeFolders(const QList<AssetsDatabase::FolderPath> &pathsToRemove) {
    
    QSet<RPZAsset::Hash> hashesToRemove;

    //traverse paths
    for(const auto &path : pathsToRemove) {

        if(!this->_paths.contains(path)) continue;

        //through assets
        hashesToRemove += this->_paths.take(path);
    
    }

    //remove referenced assets
    auto result = this->_removeAssets(hashesToRemove.values());

    //remove stored files
    this->_removeAssetFiles(result.second);

    //save
    this->save();

}

void AssetsDatabase::moveAssetsTo(const AssetsDatabase::FolderPath &internalPathToMoveTo, const QList<RPZAsset::Hash> &hashesToMove) {
    
    //remove references to assets
    auto result = this->_removeAssets(hashesToMove, true);

    //remove hashes from paths
    this->_removeHashesFromPaths(result.first);

    //add new wlink and strong link
    auto &dest = this->_paths[internalPathToMoveTo];
    for(const auto &hash : hashesToMove) {
        dest += hash;
        this->_w_assetToPath.insert(hash, internalPathToMoveTo);
    }

    //save
    this->save();

}

void AssetsDatabase::moveFoldersTo(const AssetsDatabase::FolderPath &internalPathToMoveTo, const QList<AssetsDatabase::FolderPath> &topmostPathsToMove) {
    
    auto results = this->_getPathsStartingWith(topmostPathsToMove);

    for(auto i = results.begin(); i != results.end(); i++) {
        
        auto topmost = i.key();
        auto newTopmostPath = internalPathToMoveTo + "/" + _folderName(topmost);
        
        this->_reroutePaths(topmost, newTopmostPath, i.value());

    }

    //save
    this->save();

}

////
///
///
////

void AssetsDatabase::_reroutePaths(const AssetsDatabase::FolderPath &ancestor, const AssetsDatabase::FolderPath &toReplaceAncestor, const QSet<AssetsDatabase::FolderPath> &subjects) {
    
    for(const auto &oldPath : subjects) {

        //remove old path content
        auto linkedHashes = this->_paths.take(oldPath);
        if(linkedHashes.isEmpty()) continue;
        
        //build new dest path
        auto newPath = oldPath;
        newPath.replace(ancestor, toReplaceAncestor);

        //increment dest set 
        this->_paths[newPath] += linkedHashes;

        //update wlink
        for(const auto &hash : linkedHashes) {
            this->_w_assetToPath.insert(hash, newPath);
        }

    }

}

QSet<AssetsDatabase::FolderPath> AssetsDatabase::_getPathsStartingWith(const AssetsDatabase::FolderPath &toRequest) {
    
    QSet<AssetsDatabase::FolderPath> out;
    
    for(const auto &path : this->_paths.keys()) {
        if(!path.startsWith(toRequest)) continue;
        out += path;
    }

    return out;

}

AssetsDatabase::StartingWithPathRequestResults AssetsDatabase::_getPathsStartingWith(const QList<AssetsDatabase::FolderPath> &topmostPathsToMove) {
    
    //setup request container
    StartingWithPathRequestResults startingWithPath;
    for(const auto &topmost : topmostPathsToMove) startingWithPath.insert(topmost, {});

    //iterate
    for(const auto &path : this->_paths.keys()) {
        for(const auto &topmost : topmostPathsToMove) {
            if(!path.startsWith(topmost)) continue;
            startingWithPath[topmost] += path;
        }
    }

    return startingWithPath;

}

QPair<AssetsDatabase::HashesByPathToRemove, AssetsDatabase::RemovedAssets> AssetsDatabase::_removeAssets(const QList<RPZAsset::Hash> &hashesToRemove, bool onlyRemoveReference) {
    
    RemovedAssets removedAssets;
    HashesByPathToRemove hashesToRemoveFromPaths;
    
    //clear assets
    for(const auto &hash : hashesToRemove) {

        if(!this->_assets.contains(hash)) continue;
        if(!onlyRemoveReference) removedAssets += this->_assets.take(hash);
        
        auto associatedPath = this->_w_assetToPath.take(hash);
        if(associatedPath.isEmpty()) continue;

        hashesToRemoveFromPaths[associatedPath] += hash;

    }

    return { hashesToRemoveFromPaths, removedAssets };

}

void AssetsDatabase::_removeHashesFromPaths(const HashesByPathToRemove &hashesToRemoveFromPaths) {
    for(auto i = hashesToRemoveFromPaths.begin(); i != hashesToRemoveFromPaths.end(); i++) {
        
        auto path = i.key();
        if(!this->_paths.contains(path)) continue;
        
        //substract
        auto &existingAssociatedHashes = this->_paths[path];
        existingAssociatedHashes.subtract(i.value());

        //if then empty, remove
        if(existingAssociatedHashes.isEmpty()) this->_paths.remove(path);

    }
}
        

///

const QString AssetsDatabase::_folderName(const AssetsDatabase::FolderPath &toExtractNameFrom) {
    auto toCropFrom = toExtractNameFrom.lastIndexOf(QStringLiteral(u"/"));
    auto out = toExtractNameFrom.mid(toCropFrom + 1);
    return out;
}

const QString AssetsDatabase::_parentPath(const AssetsDatabase::FolderPath &toExtractParentFrom) {

    auto toCropFrom = toExtractParentFrom.lastIndexOf(QStringLiteral(u"/"));
    
    if(toCropFrom < 0) return QStringLiteral(u"/");

    auto out = toExtractParentFrom.mid(0, toCropFrom);

    return out;
    
}

void AssetsDatabase::_removeAssetFiles(const QList<RPZAsset> &toRemoveFromStorage) {
    
    for(const auto &asset : toRemoveFromStorage) {
        
        QFile fileToRemove(asset.filepath(false));
        if(!fileToRemove.exists()) continue;
        fileToRemove.remove();
        
    }
    
}

QString AssetsDatabase::_generateNonExistingPath(const AssetsDatabase::FolderPath &parentPath, const QString &prefix) {
    
    auto newPath = parentPath;
    newPath += QStringLiteral(u"/%1_%2").arg(prefix)
                                        .arg(QDateTime().currentMSecsSinceEpoch());

    //recursive until unique
    if(this->_paths.contains(newPath)) return this->_generateNonExistingPath(parentPath, prefix);

    return newPath;

}

QHash<JSONDatabase::Version, JSONDatabase::UpdateHandler> AssetsDatabase::_getUpdateHandlers() {
    
    auto out = QHash<JSONDatabase::Version, JSONDatabase::UpdateHandler>();

    //to v5
    out.insert(
        5,
        [&](QJsonObject &doc) {
            
            AssetsDatabase db(doc);

            //iterate assets
            QVariantHash compiled;
            for(auto &asset : db._assets) {

                //try to read associated asset and define geometry
                asset.updateAssetGeometryData();
                compiled.insert(asset.hash(), asset);

            }

            //update json obj
            updateFrom(
                doc,
                QStringLiteral(u"assets"),
                compiled
            );

        }
    );

    return out;

}
