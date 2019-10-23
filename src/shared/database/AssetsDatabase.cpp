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
    
    //assets
    for(auto &i : this->entityAsObject(QStringLiteral(u"assets"))) {
        auto asset = RPZAsset(i.toVariant().toHash());
        this->_assets.insert(asset.hash(), asset);
    }

    //paths
    for(auto &i : this->entityAsObject(QStringLiteral(u"paths"))) {
        
        auto paths = i.toObject();
        
        for(auto &path : paths.keys()) {
            
            auto assetHashes = paths.value(path);
            auto uniqueHashes = assetHashes.toVariant().toStringList().toSet();

            this->_paths.insert(path, uniqueHashes);

        }

    }

}

void AssetsDatabase::_removeDatabaseLinkedFiles() {
    QDir(AppContext::getAssetsFolderLocation()).removeRecursively();
}

const RPZAsset AssetsDatabase::asset(const RPZAssetHash &hash) const {
    return this->_assets.value(hash);
}

const QSet<RPZAssetHash> AssetsDatabase::getStoredAssetsIds() const {
    return this->_assets.keys().toSet();
}


const RPZAsset* AssetsDatabase::_asset(const RPZAssetHash &hash) const {
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
    
    auto asset = this->_asset(id);
    if(!asset) return;

    RPZAssetImportPackage package(*asset);
    return package;

}

///
///
///

void AssetsDatabase::addAsset(const RPZAsset &asset, const RPZFolderPath &internalPathToAddTo) {
    
    //asset
    auto hash = asset.hash();
    this->_assets.insert(hash, asset);
    
    //path
    auto &assetsOfPath = this->_paths[internalPathToAddTo];
    assetsOfPath.insert(hash);

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

void AssetsDatabase::_renameItem(const QString &name, AssetsTreeViewItem* target) {
    
    //data template
    auto obj = this->_db.object();
    
    //prepare
    auto db_assets = this->assets();
    auto targetId = target->id();

    //find current id in db
    auto idExists = db_assets.contains(targetId);
    if(!idExists) {
        qDebug() << "Assets : cannot rename since the ID of the target cannot be found in the DB";
        return;
    }

    //save with new name
    auto asset = db_assets[targetId].toObject();
    asset[QStringLiteral(u"name")] = name;
    db_assets[targetId] = asset;
    obj[QStringLiteral(u"assets")] = db_assets;

    //update db
    this->_updateDbFile(obj);

    emit assetRenamed(target->id(), name);
}

void AssetsDatabase::_renameFolder(const QString &name, AssetsTreeViewItem* target) {

    //data template
    auto obj = this->_db.object();

    //prepare
    auto currentPath = target->path();
    auto db_paths = this->paths();

    //find current path in db
    auto pathExists = db_paths.contains(currentPath);
    if(!pathExists) {
        qDebug() << "Assets : cannot rename since the actual path to rename is not in the DB";
        return;
    }

    //update path
    auto pathAsList = AssetsTreeViewItem::pathAsList(currentPath);
    pathAsList.takeLast();
    pathAsList.append(name);
    auto updatedPath = "/" + pathAsList.join("/");

    //search for paths beeing part of the old path, and replace their value
    for(auto &path : db_paths.keys()) {
        
        //skip if not targeted
        if(!path.startsWith(currentPath)) continue; 

        //rebuild path
        auto rebuiltPath = path;
        rebuiltPath.replace(currentPath, updatedPath);

        //replace old path
        auto tempPathContent = db_paths[path].toArray();
        db_paths.remove(path);
        db_paths[rebuiltPath] = tempPathContent;
    }

    obj[QStringLiteral(u"paths")] = db_paths;

    //update db
    this->_updateDbFile(obj);
}

bool AssetsDatabase::rename(QString &name, AssetsTreeViewItem* target) { 

    //check if acceptable name change
    if(!target->isAcceptableNameChange(name)) return false;

    //rename for items
    if(target->isIdentifiable()) this->_renameItem(name, target);

    //rename for folders
    else if(target->type() == Folder) this->_renameFolder(name, target);

    //should not happend
    else return false;

    //finally rename element
    target->rename(name);
    return true;
}

////
///
///
////

void AssetsDatabase::_removeAssetFile(const RPZAssetHash &id, const QJsonObject &asset) {
    
    //prepare
    auto ext = asset[QStringLiteral(u"ext")].toString();
    auto fp = AssetsDatabase::getFilePathToAsset(id, ext);

    QFile fileToRemove(fp);

    //remove stored file
    if(fileToRemove.exists()) {
        fileToRemove.remove();
    }
    
}

QSet<RPZFolderPath> AssetsDatabase::_getPathsToAlterFromList(const QList<AssetsTreeViewItem*> &elemsToAlter) {
    
    auto out = QSet<RPZFolderPath>();
    
    for(auto &elem : elemsToAlter) {
        if(elem->isContainer()) {
            out.insert(elem->path());
        }
    }

    return out;
}

QHash<RPZFolderPath, QSet<RPZAssetHash>> AssetsDatabase::_getAssetsToAlterFromList(const QList<AssetsTreeViewItem*> &elemsToAlter) {
    auto out = QHash<RPZFolderPath, QSet<RPZAssetHash>>();
    
    for(auto &elem : elemsToAlter) {
        if(elem->isIdentifiable()) {
            out[elem->path()].insert(elem->id());
        }
    }

    return out;
}

void AssetsDatabase::_augmentAssetsHashWithMissingDescendents(QHash<RPZFolderPath, QSet<RPZAssetHash>> &hashToAugment, const QSet<RPZFolderPath> &morePathsToDelete) {
    
    auto db_paths = this->paths();

    //additional items to alter from paths arrays
    for(auto &path : morePathsToDelete) {

        //if does not contains targeted path, skip
        if(!db_paths.contains(path)) break;

        //if it does, add it to the list
        auto idsInPath = db_paths[path].toArray();
        for(auto id : idsInPath) {
            hashToAugment[path].insert(id.toString());
        }
    }

}

QList<RPZAssetHash> AssetsDatabase::_removeIdsFromPaths(QJsonObject &db_paths, const QHash<RPZFolderPath, QSet<RPZAssetHash>> &idsToRemoveByPath) {
    
    QList<RPZAssetHash> ids;

    //delete for each path
    for (auto i = idsToRemoveByPath.constBegin(); i != idsToRemoveByPath.constEnd(); ++i) { 
        
        //preapre
        auto path = i.key();
        auto idsToDelete = i.value();

        ids.append(idsToDelete.toList());

        //define ids left for the path
        auto IdsInPath = db_paths[path].toArray();
        auto idsLeftInPath = JSONDatabase::diff(
            IdsInPath, 
            idsToDelete
        );

        //if ids are still left, update the array
        if(idsLeftInPath.count()) {
            db_paths[path] = idsLeftInPath;
        } 

        // if no more ids, just delete the key
        else {
            db_paths.remove(path);
        }
    }

    return ids;
}

void AssetsDatabase::_removeAssetsFromDb(QJsonObject &db_assets, const QList<RPZAssetHash> &assetIdsToRemove) {

    QMutexLocker l(&this->_m_withAssetsElems);

    //finally delete items
    for(auto &id : assetIdsToRemove) {

        auto asset = db_assets[id].toObject();

        //remove
        db_assets.remove(id);
        
        //
        this->_withAssetsElems.remove(id);

        //remove stored file
        this->_removeAssetFile(id, asset);
    }
}

bool AssetsDatabase::removeItems(QList<AssetsTreeViewItem*> elemsToRemove) {

    //prepare
    auto db_paths = this->paths();
    auto db_assets = this->assets();

    //compartimentation
    auto pathsToDelete = this->_getPathsToAlterFromList(elemsToRemove);
    auto idsToRemoveByPath = this->_getAssetsToAlterFromList(elemsToRemove);

    //augment list of paths to alter with children of paths already planned for alteration
    auto morePathsToDelete = this->_augmentPathsSetWithMissingDescendents(pathsToDelete);

    //since we augmented the paths, also impacts ids
    this->_augmentAssetsHashWithMissingDescendents(idsToRemoveByPath, morePathsToDelete);
    
    //remove ids from paths
    auto removedIdsFromPaths = this->_removeIdsFromPaths(db_paths, idsToRemoveByPath);

    //delete remaining paths, e.g. empty folders not auto-deleted by removeIdsFromPaths()
    for(auto &path : pathsToDelete) {
        if(db_paths.contains(path)) {
            db_paths.remove(path);
        }
    }
    
    //delete assets
    this->_removeAssetsFromDb(db_assets, removedIdsFromPaths);

    //save changes
    auto obj = this->_db.object();
    obj[QStringLiteral(u"assets")] = db_assets;
    obj[QStringLiteral(u"paths")] = db_paths;
    this->_updateDbFile(obj);

    //sort items by path length (number of slashes); delete them first
    AssetsTreeViewItem::sortByPathLengthDesc(elemsToRemove);
    for(auto elem : elemsToRemove) {
        //finally delete elems
        delete elem;
    }

    return true;
}



bool AssetsDatabase::moveItemsToContainer(QList<AssetsTreeViewItem*> selectedItemsToMove, AssetsTreeViewItem* target) {
    
    auto db_paths = this->paths();

    //reduce targetedItems list to only the higher in order for each node
    auto higherElems = AssetsTreeViewItem::filterTopMostOnly(selectedItemsToMove).toList();

    //move assets
    auto idsToMove = this->_getAssetsToAlterFromList(higherElems);
    auto idsRemoved = this->_removeIdsFromPaths(db_paths, idsToMove);

        //bind ids to target
        auto targetPath = target->path();
        auto target_db_path = db_paths[targetPath].toArray();
        for(auto &id : idsRemoved) {
            target_db_path.append(id);
        }
        db_paths[targetPath] = target_db_path;

    
    //rename paths
    auto pathsToMove = this->_getPathsToAlterFromList(higherElems);

    auto parentPath = target->path();

    for(auto &pathToMove : pathsToMove) {

        //for each higher elem, augment with descendants
        auto augmentedList = QSet<RPZFolderPath> { pathToMove };
        this->_augmentPathsSetWithMissingDescendents(augmentedList);

        //define new path
        auto displayName = AssetsTreeViewItem::pathAsList(pathToMove).takeLast();
        auto newPath = parentPath + "/" + displayName;

        for(auto &path : augmentedList) {
            
            //generate the dest path
            auto newFullPath = path;
            newFullPath.replace(pathToMove, newPath);

            //get/create the dest path in db
            auto dest = db_paths[newFullPath].toArray();

            //if there are ids to transfert
            auto content = db_paths[path].toArray();
            if(content.count()) {
                for(auto id : content) {
                    dest.append(id);
                }
            }

            //erase new path array by new one
            db_paths[newFullPath] = dest;

            //remove old path
            db_paths.remove(path);
        }
        
    }

    //update db
    auto obj = this->_db.object();
    obj[QStringLiteral(u"paths")] = db_paths;
    this->_updateDbFile(obj);

    //update model
    for(auto item : higherElems) {
        target->appendChild(item);
    }

    return true;
}

QSet<RPZFolderPath> AssetsDatabase::_augmentPathsSetWithMissingDescendents(QSet<RPZFolderPath> &setToAugment) {
        
    //remove already planned alterations
    QSet<RPZFolderPath> inheritedPathAlterations;
    auto remaining_db_paths = this->paths().keys().toSet().subtract(setToAugment);

    //check if a path to be remove is a part of a remaining path 
    for(auto &remaining_path : remaining_db_paths) {
        
        for(auto &pathToCompareTo : setToAugment) {
            
            //if so, adds it to alteration list
            if(remaining_path.startsWith(pathToCompareTo)) {
                inheritedPathAlterations.insert(remaining_path);
            }

        }
    }

    //fusion of sets
    for(auto &path : inheritedPathAlterations) {
        setToAugment.insert(path);
    }

    return inheritedPathAlterations;
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
