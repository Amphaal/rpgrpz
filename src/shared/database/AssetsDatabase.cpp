#include "AssetsDatabase.h"

AssetsDatabase* AssetsDatabase::get() {
    if(!_singleton) {
        _singleton = new AssetsDatabase();
    }
    return _singleton;
}

AssetsDatabase::AssetsDatabase() { 
    this->_instanciateDb();
    this->_injectStaticStructure();
    this->_injectDbStructure();
};

const int AssetsDatabase::dbVersion() {
    return this->_db["version"].toInt();
}

const int AssetsDatabase::apiVersion() {
    return 2;
}

void AssetsDatabase::_removeDatabase() {
    JSONDatabase::_removeDatabase();
    QDir(this->assetsStorageFilepath()).removeRecursively();
}

const QString AssetsDatabase::defaultJsonDoc() {
    return "{\"version\":" + QString::number(this->apiVersion()) + ",\"paths\":{},\"assets\":{}}";
};
const QString AssetsDatabase::dbPath() {
    return AppContext::getAssetsFileCoordinatorLocation();
};

QString AssetsDatabase::assetsStorageFilepath() {
    return AppContext::getAssetsFolderLocation();
}


QString AssetsDatabase::importAsset(const QVariantHash &package) {
    
    if(package.isEmpty()) return NULL;
    
    //check content
    auto fileBytes = package["_fileContent"].toByteArray();
    if(!fileBytes.size()) return NULL;

    //asset
    auto asset_id = package["_id"].toString();
    auto asset = QJsonDocument::fromVariant(package["_meta"]).object();

    //save file
    auto fileAsRawData = QByteArray::fromBase64(fileBytes);
    auto destFileExt = asset["ext"].toString();
    auto destFileName = asset["name"].toString();
    auto destUrl = this->_moveFileToDbFolder(fileAsRawData, destFileExt, destFileName, asset_id);

    //copy into db
    auto parent = this->_staticElements[DownloadedContainer];
    this->_addAssetToDb(asset_id, destUrl, parent);

    //add to tree
    auto element = new AssetsDatabaseElement(destFileName, parent, parent->insertType(), asset_id);
    qDebug() << "Assets :" << asset_id << "imported";
    return asset_id;
}

QVariantHash AssetsDatabase::prepareAssetPackage(QString &assetId) {
    
    //determine id existance by fetching the file path
    auto pathToFile = this->getFilePathToAsset(assetId);
    if(pathToFile.isNull()) return QVariantHash();
    
    //json obj
    QVariantHash package;
    package["_id"] = assetId;

    //append file as base64
    QFile assetFile(pathToFile);
    assetFile.open(QFile::ReadOnly);

        auto fileContent = assetFile.readAll().toBase64();
        package["_fileContent"] = QString(fileContent);

    assetFile.close();

    package["_meta"] = this->assets()[assetId].toObject();
    return package;
}

QString AssetsDatabase::getFilePathToAsset(QString &assetId) {
    auto db_assets = this->assets();
    if(!db_assets.contains(assetId)) return NULL;

    auto assetJSON = db_assets[assetId].toObject();
    auto fileName = assetId + "." + assetJSON["ext"].toString();

    return this->assetsStorageFilepath() + "/" + fileName;
}

QString AssetsDatabase::getFilePathToAsset(AssetsDatabaseElement* asset) {
    return this->getFilePathToAsset(asset->id());
}

///
///
///

QJsonObject AssetsDatabase::paths() {
    return this->_db["paths"].toObject();
}

QJsonObject AssetsDatabase::assets() {
    return this->_db["assets"].toObject();
}

///
///
///

void AssetsDatabase::_injectStaticStructure() {

    for(auto &staticType : AssetsDatabaseElement::staticContainerTypes()) {

        auto staticFolder = new AssetsDatabaseElement(AssetsDatabaseElement::typeDescription(staticType), this, staticType);
        this->_staticElements.insert(staticType, staticFolder);

        if(staticType == InternalContainer) {
            for(auto &type : AssetsDatabaseElement::internalItemTypes()) {
                auto internalItem = new AssetsDatabaseElement(AssetsDatabaseElement::typeDescription(type), staticFolder, type);
            }
        }
    }

}


void AssetsDatabase::_injectDbStructure() {

    //get containers to fill, not inbetween folders
    auto containersToFill = this->_generateFolderTreeFromDb();

    //then fill with items
    this->_generateItemsFromDb(containersToFill);

}

QHash<QString, AssetsDatabaseElement*> AssetsDatabase::_generateFolderTreeFromDb() {
    
    //sort the keys
    auto paths = this->paths().keys();
    paths.sort();

    //to be created items
    QHash<QString, AssetsDatabaseElement*> containersToFill;

    //create folders arbo
    for(auto &path : paths) {
        
        //split the path
        auto split = AssetsDatabaseElement::pathAsList(path);
        
        //make sure first split is a type
        auto staticCType = AssetsDatabaseElement::pathChunktoType(split.takeFirst());

        //get element from static source
        if(!this->_staticElements.contains(staticCType)) {
            qDebug() << "Assets : ignoring path, as the static container it points to doesnt exist";
            continue;
        }
        auto staticContainerElem = this->_staticElements[staticCType];

        //create path
        auto lastContainer = this->_recursiveElementCreator(staticContainerElem, split);
        
        //append to list
        containersToFill.insert(path, lastContainer);
    }

    //returns
    return containersToFill;
}

AssetsDatabaseElement* AssetsDatabase::_recursiveElementCreator(AssetsDatabaseElement* parent, QList<QString> pathChunks) {
    
    //if no more folders in path to create...
    if(!pathChunks.count()) return parent;

    //take first part
    auto part = pathChunks.takeFirst();

    //search if already exist
    AssetsDatabaseElement* found = nullptr;
    for(auto container : parent->childrenContainers()) {
        if(container->displayName() == part) {
            found = container;
            break;
        }
    }

    //if not found, create it
    if(!found) {
        found = new AssetsDatabaseElement(part, parent);
    }

    //iterate through...
    return this->_recursiveElementCreator(found, pathChunks);

}

void AssetsDatabase::_generateItemsFromDb(QHash<QString, AssetsDatabaseElement*> &pathsToFillWithItems) {
    
    auto db_paths = this->paths();
    auto assets_db = this->assets();

    //create items for each end-containers
    QHash<QString, AssetsDatabaseElement*>::iterator i;
    for (i = pathsToFillWithItems.begin(); i != pathsToFillWithItems.end(); ++i) {
        
        //define
        auto path = i.key();
        auto lastContainer = i.value();

        //preapre for search
        auto items_ids = db_paths[path].toArray();
        
        //find items in db and create them
        for(auto &id : items_ids) {
            
            auto idStr = id.toString();

            //if ID doesnt exist
            if(!assets_db.contains(idStr)) {
                qDebug() << "Assets : cannot insert an item as its ID is not found in the assets DB";
                continue;
            }

            //create
            auto asset = assets_db[idStr].toObject();
            auto assetName = asset["name"].toString();
            auto newElem = new AssetsDatabaseElement(assetName, lastContainer, lastContainer->insertType(), idStr);
        }
    }

}


///
///
///

QString AssetsDatabase::_getFileSignatureFromFileUri(QUrl &url) {
    
    if(!url.isLocalFile()) {
        qDebug() << "Assets : cannot insert, uri is not a file !";
        return NULL;
    }

    //check file exists
    auto sourceFile = QFile(url.toLocalFile());
    if(!sourceFile.exists()) {
        qDebug() << "Assets : cannot insert, uri file does not exist !";
        return NULL;
    }

    sourceFile.open(QFile::ReadOnly);
        
        //read signature...
        auto signature = QString::fromUtf8(
            QCryptographicHash::hash(sourceFile.readAll(), QCryptographicHash::Keccak_224).toHex()
        );

    sourceFile.close();

    return signature;
}

QUrl AssetsDatabase::_moveFileToDbFolder(QByteArray &data, QString &fileExt, QString &name, QString &id) {
    
    //turn encoded file from JSON into file
    auto destFolder = this->assetsStorageFilepath();
    auto dest = destFolder + "/" + id + "." + fileExt;
    auto forgedDest = destFolder + "/" + name + "." + fileExt;

    //write file
    QFile assetFile(dest);
    assetFile.open(QFile::WriteOnly);
        auto written = assetFile.write(data);
    assetFile.close();
    
    //dummy
    return QUrl(forgedDest);
}

bool AssetsDatabase::_moveFileToDbFolder(QUrl &url, QString &id) {

    //dest file suffix
    QFileInfo fInfo(url.fileName());
    auto destFileExt = fInfo.suffix();
    auto destFileName = fInfo.baseName();
    
    //file as raw data
    QFile assetFile(url.fileName());
    assetFile.open(QFile::ReadOnly);
        auto data = assetFile.readAll();
    assetFile.close();

    //
    auto destUrl = this->_moveFileToDbFolder(data, destFileExt, destFileName, id);
    return !destUrl.isEmpty();
}

QString AssetsDatabase::_addAssetToDb(QString &id, QUrl &url, AssetsDatabaseElement* parent) {

    //prepare
    auto db_paths = this->paths();
    auto obj = this->_db.object();
    auto folderParentPath = parent->path();
    QFileInfo fInfo(url.fileName());

    //1.save new asset
    auto assets = this->assets();

        //define new asset item
        auto newAsset = QJsonObject();
        newAsset["ext"] = fInfo.suffix();
        newAsset["name"] = fInfo.baseName();

    assets.insert(id, newAsset);
    obj["assets"] = assets;
        

    //check path existance, create it if non existant
    QJsonArray objsInPath = db_paths.contains(folderParentPath) ? db_paths[folderParentPath].toArray() : QJsonArray();
    if(!objsInPath.contains(id)) {

        //2. update path status
        objsInPath.append(id);
        db_paths[folderParentPath] = objsInPath;
        obj["paths"] = db_paths;
    }


    //update db file
    this->_updateDbFile(obj);
    qDebug() << "Assets : " << fInfo.filePath() << " inserted !";
    
    return fInfo.baseName();
}

bool AssetsDatabase::insertAsset(QUrl &url, AssetsDatabaseElement* parent) {

    //get the corresponding signature
    auto fileSignature = this->_getFileSignatureFromFileUri(url);
    if(fileSignature.isNull()) return false;

    // comparaison from db
    auto fileSignatures = this->assets().keys();
    if(fileSignatures.contains(fileSignature)) {
        qDebug() << "Assets : will not insert, file signature has been found in db !";
        return false;
    }

    //copy file
    if(!this->_moveFileToDbFolder(url, fileSignature)) return false;

    //insert into db
    auto displayName = this->_addAssetToDb(fileSignature, url, parent);

    //add element
    auto element = new AssetsDatabaseElement(displayName, parent, parent->insertType(), fileSignature);
    return true;
}


bool AssetsDatabase::createFolder(AssetsDatabaseElement* parent) {
    
    //data template
    auto generatedPath = this->_generateNonExistingPath(parent, "Dossier");

    //save
    auto obj = this->_db.object();
    auto db_paths = this->paths();
    db_paths[generatedPath] = QJsonArray();
    obj["paths"] = db_paths;
    this->_updateDbFile(obj);

    //create elem
    auto generatedName = AssetsDatabaseElement::pathAsList(generatedPath).takeLast();
    auto folder = new AssetsDatabaseElement(generatedName, parent);

    return true;
}

QString AssetsDatabase::_generateNonExistingPath(AssetsDatabaseElement* parent, QString prefix) {
    
    QString generatedPath = "";
    QString destPath = parent->path();
    
    //prepare path generation
    auto generateNewPath = [destPath, prefix]() {
        auto newPath = destPath;
        newPath += "/" + prefix + "_";
        newPath += QString::number(QDateTime().currentMSecsSinceEpoch());
        return newPath;
    }; 

    //generate non existing path in db
    auto db_paths = this->paths();
    do {
        generatedPath = generateNewPath();
    } while(db_paths.contains(generatedPath));

    //returns
    return generatedPath;
}

void AssetsDatabase::_renameItem(QString &name, AssetsDatabaseElement* target) {
    
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
    asset["name"] = name;
    db_assets[targetId] = asset;
    obj["assets"] = db_assets;

    //update db
    this->_updateDbFile(obj);
}

void AssetsDatabase::_renameFolder(QString &name, AssetsDatabaseElement* target) {

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
    auto pathAsList = AssetsDatabaseElement::pathAsList(currentPath);
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

    obj["paths"] = db_paths;

    //update db
    this->_updateDbFile(obj);
}

bool AssetsDatabase::rename(QString name, AssetsDatabaseElement* target) { 

    //check if acceptable name change
    if(!target->isAcceptableNameChange(name)) return false;

    //rename for items
    if(target->isItem()) this->_renameItem(name, target);

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

void AssetsDatabase::_removeAssetFile(QString &id, QJsonObject &asset) {
    
    //prepare
    auto fileName = id + "." + asset["ext"].toString();

    auto expectedLocation = this->assetsStorageFilepath();
    auto fileToRemove = QFile(expectedLocation + "/" + fileName);

    //remove stored file
    if(fileToRemove.exists()) {
        fileToRemove.remove();
    }
}

QSet<QString> AssetsDatabase::_getPathsToAlterFromList(QList<AssetsDatabaseElement*> &elemsToAlter) {
    
    auto out = QSet<QString>();
    
    for(auto &elem : elemsToAlter) {
        if(elem->isContainer()) {
            out.insert(elem->path());
        }
    }

    return out;
}

QHash<QString, QSet<QString>> AssetsDatabase::_getAssetsToAlterFromList(QList<AssetsDatabaseElement*> &elemsToAlter) {
    auto out = QHash<QString, QSet<QString>>();
    
    for(auto &elem : elemsToAlter) {
        if(elem->isItem()) {
            out[elem->path()].insert(elem->id());
        }
    }

    return out;
}

void AssetsDatabase::_augmentAssetsHashWithMissingDescendents(QHash<QString, QSet<QString>> &hashToAugment, QSet<QString> &morePathsToDelete) {
    
    auto db_paths = this->paths();

    //additional items to alter from paths arrays
    for(auto &path : morePathsToDelete) {

        //if does not contains targeted path, skip
        if(!db_paths.contains(path)) break;

        //if it does, add it to the list
        for(auto &id : db_paths[path].toArray()) {
            hashToAugment[path].insert(id.toString());
        }
    }

}

QList<QString> AssetsDatabase::_removeIdsFromPaths(QJsonObject &db_paths, QHash<QString, QSet<QString>> &idsToRemoveByPath) {
    
    QList<QString> ids;

    //delete for each path
    QHash<QString, QSet<QString>>::iterator i;
    for (i = idsToRemoveByPath.begin(); i != idsToRemoveByPath.end(); ++i) { 
        
        //preapre
        auto path = i.key();
        auto idsToDelete = i.value();

        ids.append(idsToDelete.toList());

        //define ids left for the path
        auto idsLeftInPath = JSONDatabase::diff(
            db_paths[path].toArray(), 
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

void AssetsDatabase::_removeAssetsFromDb(QJsonObject &db_assets, QList<QString> &assetIdsToRemove) {

    //finally delete items
    for(auto &id : assetIdsToRemove) {

        auto asset = db_assets[id].toObject();

        //remove
        db_assets.remove(id);

        //remove stored file
        this->_removeAssetFile(id, asset);
    }
}

bool AssetsDatabase::removeItems(QList<AssetsDatabaseElement*> elemsToRemove) {

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
    obj["assets"] = db_assets;
    obj["paths"] = db_paths;
    this->_updateDbFile(obj);

    //sort items by path length (number of slashes); delete them first
    AssetsDatabaseElement::sortByPathLengthDesc(elemsToRemove);
    for(auto elem : elemsToRemove) {
        //finally delete elems
        delete elem;
    }

    return true;
}



bool AssetsDatabase::moveItems(QList<AssetsDatabaseElement*> selectedItemsToMove, AssetsDatabaseElement* target) {
    
    auto db_paths = this->paths();

    //reduce targetedItems list to only the higher in order for each node
    auto higherElems = AssetsDatabaseElement::filterTopMostOnly(selectedItemsToMove).toList();

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
        auto augmentedList = QSet<QString> { pathToMove };
        this->_augmentPathsSetWithMissingDescendents(augmentedList);

        //define new path
        auto displayName = AssetsDatabaseElement::pathAsList(pathToMove).takeLast();
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
                for(auto &id : content) {
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
    obj["paths"] = db_paths;
    this->_updateDbFile(obj);

    //update model
    for(auto &item : higherElems) {
        target->appendChild(item);
    }

    return true;
}

QSet<QString> AssetsDatabase::_augmentPathsSetWithMissingDescendents(QSet<QString> &setToAugment) {
        
    //remove already planned alterations
    QSet<QString> inheritedPathAlterations;
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
