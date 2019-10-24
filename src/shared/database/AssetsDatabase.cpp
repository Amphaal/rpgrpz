#include "AssetsDatabase.h"

AssetsDatabase::AssetsDatabase(const QJsonObject &doc) : JSONDatabase(doc) { }
AssetsDatabase::AssetsDatabase() : JSONDatabase(AppContext::getAssetsFileCoordinatorLocation()) {};

AssetsDatabase* AssetsDatabase::get() {
    if(!_singleton) {
        _singleton = new AssetsDatabase();
    }
    return _singleton;
}

const JSONDatabase::Version AssetsDatabase::apiVersion() {
    return 5;
}

JSONDatabase::Model AssetsDatabase::_getDatabaseModel() {
    return {
        { { QStringLiteral(u"paths"), JSONDatabase::EntityType::Object }, &this->_paths },
        { { QStringLiteral(u"assets"), JSONDatabase::EntityType::Object }, &this->_assets }
    };
}

void AssetsDatabase::_saveIntoFile() {
    
    auto db = this->db();

        //assets
        QVariantHash assets;
        for(auto &asset : this->_assets) {
            assets.insert(asset.hash(), asset);
        }
        updateFrom(db, QStringLiteral(u"assets"), assets);

        //paths
        QVariantMap paths;
        for(auto i = this->_paths.begin(); i != this->_paths.end(); i++) {
            
            QVariantList hashes;
            for(auto &hash : i.value()) hashes += hash;

            paths.insert(i.key(), hashes);

        }
        updateFrom(db, QStringLiteral(u"paths"), paths);

    this->_updateDbFile(db);

};

void AssetsDatabase::_setupLocalData() {

    //paths
    for(auto i : this->entityAsObject(QStringLiteral(u"paths"))) {
        
        auto paths = i.toObject();
        
        for(auto &path : paths.keys()) {
            
            auto assetHashes = paths.value(path);
            auto uniqueHashes = assetHashes.toVariant().toStringList().toSet();

            this->_paths.insert(path, uniqueHashes);

        }

    }

    //assets
    for(auto i : this->entityAsObject(QStringLiteral(u"assets"))) {
        auto asset = RPZAsset(i.toVariant().toHash());
        this->_assets.insert(asset.hash(), asset);
    }

    //wlink, internal only
    for(auto i = this->_paths.begin(); i != this->_paths.end(); i++) {
        auto path = i.key();
        for(auto &hashes : i.value()) {
            this->_w_assetToPath.insert(hashes, i.key());
        }
    }


}

void AssetsDatabase::_removeDatabaseLinkedFiles() {
    QDir(AppContext::getAssetsFolderLocation()).removeRecursively();
}

const QSet<RPZAssetHash> AssetsDatabase::getStoredAssetsIds() const {
    return this->_assets.keys().toSet();
}

const QMap<RPZFolderPath, QSet<RPZAssetHash>> AssetsDatabase::paths() const {
    return this->_paths;
}

const QHash<RPZAssetHash, RPZAsset> AssetsDatabase::assets() const {
    return this->_assets;
}

const RPZAsset* AssetsDatabase::asset(const RPZAssetHash &hash) {
    if(!this->_assets.contains(hash)) {
        qDebug() << "Assets: cannot find asset !";
        return nullptr;
    }
    return &this->_assets[hash];
}

RPZAsset* AssetsDatabase::_asset(const RPZAssetHash &hash) {
    if(!this->_assets.contains(hash)) {
        qDebug() << "Assets: cannot find asset !";
        return nullptr;
    }
    return &this->_assets[hash];
}

const QString AssetsDatabase::_path(const StorageContainer &targetContainer) const {
    auto path = QStringLiteral(u"/{%1}").arg(QString::number((int)targetContainer));
    return path;
}

const RPZAssetImportPackage AssetsDatabase::prepareAssetPackage(const RPZAssetHash &hash) const {
    
    if(!this->_assets.contains(hash)) return RPZAssetImportPackage();

    RPZAssetImportPackage package(this->_assets.value(hash));
    return package;

}

///
///
///

void AssetsDatabase::addAsset(const RPZAsset &asset, const RPZFolderPath &internalPathToAddTo) {
    
    //asset
    auto hash = asset.hash();
    this->_assets.insert(hash, asset);

    //weak link
    this->_w_assetToPath.insert(hash, internalPathToAddTo);
    
    //path
    auto &assetsOfPath = this->_paths[internalPathToAddTo];
    assetsOfPath.insert(asset.hash());

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

void AssetsDatabase::createFolder(const RPZFolderPath &parentPath) {
    
    //get a unique folder
    auto uniquePath = this->_generateNonExistingPath(parentPath, QObject::tr("Folder"));

    //add
    this->_paths.insert(uniquePath, {});

}

bool AssetsDatabase::renameFolder(const QString &requestedNewFolderName, const RPZFolderPath &pathToRename) {
    
    //prevent slashes
    if(requestedNewFolderName.contains(QStringLiteral(u"/"))) return false;

    //check if path already exists
    auto toRequest = _parentPath(pathToRename) + requestedNewFolderName;
    if(this->_paths.contains(toRequest)) return false;

    //get all paths starting with renamed path
    auto oldPathsToReroute = this->_getPathsStartingWith(pathToRename);
    this->_reroutePaths(pathToRename, toRequest, oldPathsToReroute);

    return true;

}

void AssetsDatabase::renameAsset(const QString &newName, const RPZAssetHash &hash) {
    
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

}

void AssetsDatabase::removeAssets(const QList<RPZAssetHash> &hashesToRemove) {
    
    //clear assets
    auto result = this->_removeAssets(hashesToRemove);

    //clear paths
    this->_removeHashesFromPaths(result.first);

    //remove stored files
    this->_removeAssetFiles(result.second);

}

void AssetsDatabase::removeFolders(const QList<RPZFolderPath> &pathsToRemove) {
    
    QSet<RPZAssetHash> hashesToRemove;

    //traverse paths
    for(auto &path : pathsToRemove) {

        if(!this->_paths.contains(path)) continue;

        //through assets
        hashesToRemove += this->_paths.take(path);
    
    }

    //remove referenced assets
    auto result = this->_removeAssets(hashesToRemove.toList());

    //remove stored files
    this->_removeAssetFiles(result.second);

}

void AssetsDatabase::moveAssetsTo(const RPZFolderPath &internalPathToMoveTo, const QList<RPZAssetHash> &hashesToMove) {
    
    //remove references to assets
    auto result = this->_removeAssets(hashesToMove, true);

    //remove hashes from paths
    this->_removeHashesFromPaths(result.first);

    //add new wlink and strong link
    auto &dest = this->_paths[internalPathToMoveTo];
    for(auto &hash : hashesToMove) {
        dest += hash;
        this->_w_assetToPath.insert(hash, internalPathToMoveTo);
    }

}

void AssetsDatabase::moveFoldersTo(const RPZFolderPath &internalPathToMoveTo, const QList<RPZFolderPath> &topmostPathsToMove) {
    
    auto results = this->_getPathsStartingWith(topmostPathsToMove);

    for(auto i = results.begin(); i != results.end(); i++) {
        
        auto topmost = i.key();
        auto newTopmostPath = internalPathToMoveTo + "/" + _folderName(topmost);
        
        this->_reroutePaths(topmost, newTopmostPath, i.value());

    }

}

////
///
///
////

void AssetsDatabase::_reroutePaths(const RPZFolderPath &ancestor, const RPZFolderPath &toReplaceAncestor, const QSet<RPZFolderPath> &subjects) {
    
    for(auto &oldPath : subjects) {

        //remove old path content
        auto linkedHashes = this->_paths.take(oldPath);
        if(linkedHashes.isEmpty()) continue;
        
        //build new dest path
        auto newPath = oldPath;
        newPath.replace(ancestor, toReplaceAncestor);

        //increment dest set 
        this->_paths[newPath] += linkedHashes;

        //update wlink
        for(auto &hash : linkedHashes) {
            this->_w_assetToPath.insert(hash, newPath);
        }

    }

}

QSet<RPZFolderPath> AssetsDatabase::_getPathsStartingWith(const RPZFolderPath &toRequest) {
    
    QSet<RPZFolderPath> out;
    
    for(auto &path : this->_paths.keys()) {
        if(!path.startsWith(toRequest)) continue;
        out += path;
    }

    return out;

}

AssetsDatabase::StartingWithPathRequestResults AssetsDatabase::_getPathsStartingWith(const QList<RPZFolderPath> &topmostPathsToMove) {
    
    //setup request container
    StartingWithPathRequestResults startingWithPath;
    for(auto &topmost : topmostPathsToMove) startingWithPath.insert(topmost, {});

    //iterate
    for(auto &path : this->_paths.keys()) {
        for(auto &topmost : topmostPathsToMove) {
            if(!path.startsWith(topmost)) continue;
            startingWithPath[topmost] += path;
        }
    }

    return startingWithPath;

}

QPair<AssetsDatabase::HashesByPathToRemove, AssetsDatabase::RemovedAssets> AssetsDatabase::_removeAssets(const QList<RPZAssetHash> &hashesToRemove, bool onlyRemoveReference) {
    
    RemovedAssets removedAssets;
    HashesByPathToRemove hashesToRemoveFromPaths;
    
    //clear assets
    for(auto &hash : hashesToRemove) {

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

const QString AssetsDatabase::_folderName(const RPZFolderPath &toExtractNameFrom) {
    auto toCropFrom = toExtractNameFrom.lastIndexOf(QStringLiteral(u"/"));
    auto out = toExtractNameFrom.mid(toCropFrom + 1);
    return out;
}

const QString AssetsDatabase::_parentPath(const RPZFolderPath &toExtractParentFrom) {

    auto toCropFrom = toExtractParentFrom.lastIndexOf(QStringLiteral(u"/"));
    
    if(toCropFrom < 0) return QStringLiteral(u"/");

    auto out = toExtractParentFrom.mid(0, toCropFrom);

    return out;
    
}

void AssetsDatabase::_removeAssetFiles(const QList<RPZAsset> &toRemoveFromStorage) {
    
    for(auto &asset : toRemoveFromStorage) {
        QFile fileToRemove(asset.filepath());
        if(!fileToRemove.exists()) continue;
        fileToRemove.remove();
    }
    
}

QString AssetsDatabase::_generateNonExistingPath(const RPZFolderPath &parentPath, const QString &prefix) {
    
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
