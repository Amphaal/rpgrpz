#include "AssetsDatabase.h"

AssetsDatabase::AssetsDatabase(const QJsonObject &doc) : JSONDatabase(doc) { }
AssetsDatabase::AssetsDatabase() : JSONDatabase(AppContext::getAssetsFileCoordinatorLocation()) {};

AssetsDatabase* AssetsDatabase::get() {
    if(!_singleton) {
        _singleton = new AssetsDatabase();
    }
    return _singleton;
}

const int AssetsDatabase::apiVersion() {
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
    for(auto &i : this->entityAsObject(QStringLiteral(u"paths"))) {
        
        auto paths = i.toObject();
        
        for(auto &path : paths.keys()) {
            
            auto assetHashes = paths.value(path);
            auto uniqueHashes = assetHashes.toVariant().toStringList().toSet();

            this->_paths.insert(path, uniqueHashes);

        }

    }

    //assets
    for(auto &i : this->entityAsObject(QStringLiteral(u"assets"))) {
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

const RPZAssetImportPackage AssetsDatabase::prepareAssetPackage(const RPZAssetHash &id) const {
    
    auto asset = this->asset(id);
    if(asset.isEmpty()) return;

    RPZAssetImportPackage package(asset);
    return package;

}

///
///
///

void AssetsDatabase::_addAsset(const RPZAsset &asset, const RPZFolderPath &internalPathToAddTo) {
    
    //asset
    auto hash = asset.hash();
    this->_assets.insert(hash, asset);

    //weak link
    this->_w_assetToPath.insert(hash, internalPathToAddTo);

}

void AssetsDatabase::addAsset(const RPZAsset &asset, const RPZFolderPath &internalPathToAddTo) {
    
    //asset
    this->_addAsset(asset, internalPathToAddTo);
    
    //path
    auto &assetsOfPath = this->_paths[internalPathToAddTo];
    assetsOfPath.insert(asset.hash());

    //save
    this->_saveIntoFile();

}

void AssetsDatabase::importAsset(RPZAssetImportPackage &package) {
    
    auto success = package.tryIntegratePackage();
    if(!success) return;

    this->addAsset(
        package, 
        this->_path(StorageContainer::Downloaded)
    );

}

void AssetsDatabase::createFolder(const RPZFolderPath &parentPath) {
    
    //get a unique folder
    auto uniquePath = this->_generateNonExistingPath(parentPath, tr("Folder"));

    //add
    this->_paths.insert(uniquePath, {});

    //save
    this->_saveIntoFile();

}

bool AssetsDatabase::renameFolder(const QString &requestedNewFolderName, const RPZFolderPath &pathToRename) {
    
    //prevent slashes
    if(requestedNewFolderName.contains(QStringLiteral(u"/"))) return false;

    //check if path already exists
    auto toRequest = this->_parentPath(pathToRename) + requestedNewFolderName;
    if(this->_paths.contains(toRequest)) return false;

    //apply
    auto linkedHashes = this->_paths.take(pathToRename);
    this->_paths.insert(toRequest, linkedHashes);

    //save
    this->_saveIntoFile();

}

void AssetsDatabase::renameAsset(const QString &newName, const RPZAssetHash &hash) {
    
    //find
    auto assetToRename = this->_asset(hash);
    if(!assetToRename) return;

    //apply
    assetToRename->rename(newName);

    //save
    this->_saveIntoFile();

}

void AssetsDatabase::removeAssets(const QList<RPZAssetHash> &hashesToRemove) {
    
    QList<RPZAsset> removedAssets;
    QHash<RPZFolderPath, QSet<RPZAssetHash>> hashesToRemoveFromPaths;
    
    //clear assets
    for(auto &hash : hashesToRemove) {

        if(!this->_assets.contains(hash)) continue;
        removedAssets += this->_assets.take(hash);
        this->_w_assetToPath.remove(hash);
        
        auto associatedPath = this->_w_assetToPath.value(hash);
        if(associatedPath.isEmpty()) continue;

        hashesToRemoveFromPaths[associatedPath] += hash;

    }

    //clear paths
    for(auto i = hashesToRemoveFromPaths.begin(); i != hashesToRemoveFromPaths.end(); i++) {
        
        auto path = i.key();
        if(!this->_paths.contains(path)) continue;
        
        //substract
        auto &existingAssociatedHashes = this->_paths[path];
        existingAssociatedHashes.subtract(i.value());

        //if then empty, remove
        if(existingAssociatedHashes.isEmpty()) this->_paths.remove(path);

    }

    //remove stored files
    this->_removeAssetFiles(removedAssets);

    //save
    this->_saveIntoFile();

}

void AssetsDatabase::removeFolders(const QList<RPZFolderPath> &pathsToRemove) {
    
    QList<RPZAsset> removedAssets;

    //traverse paths
    for(auto &path : pathsToRemove) {

        if(!this->_paths.contains(path)) continue;

        //through assets
        auto hashesToRemove = this->_paths.take(path);
        for(auto &hash : hashesToRemove) {
            
            if(!this->_assets.contains(hash)) continue;

            removedAssets += this->_assets.take(hash);
            this->_w_assetToPath.remove(hash);

        }

    }

    //remove stored files
    this->_removeAssetFiles(removedAssets);

    //save
    this->_saveIntoFile();
}

void AssetsDatabase::moveAssetsTo(const RPZFolderPath &internalPathToMoveTo, const QList<RPZAssetHash> &hashesToMove) {
    //TODO
}

void AssetsDatabase::moveFoldersTo(const RPZFolderPath &internalPathToMoveTo, const QList<RPZFolderPath> &pathsToMove) {
    //TODO
}

////
///
///
////

const QString _parentPath(const RPZFolderPath &toExtractParentFrom) {

    auto toCropFrom = toExtractParentFrom.lastIndexOf(QStringLiteral(u"/"));
    
    if(toCropFrom < 0) return QStringLiteral(u"/");

    auto out = toExtractParentFrom.mid(0, toCropFrom);
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
