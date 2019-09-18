#include "AssetsDatabase.h"

AssetsDatabase* AssetsDatabase::get() {
    if(!_singleton) {
        _singleton = new AssetsDatabase();
    }
    return _singleton;
}

AssetsDatabase::AssetsDatabase() : QObject(nullptr) { 
    this->_instanciateDb();
    this->_injectStaticStructure();
    this->_injectDbStructure();
};

const int AssetsDatabase::apiVersion() {
    return 5;
}

void AssetsDatabase::_removeDatabaseLinkedFiles() {
    QDir(AppContext::getAssetsFolderLocation()).removeRecursively();
}

const QString AssetsDatabase::defaultJsonDoc() {
    return "{\"version\":" + QString::number(this->apiVersion()) + ",\"paths\":{},\"assets\":{}}";
};
const QString AssetsDatabase::dbPath() {
    return AppContext::getAssetsFileCoordinatorLocation();
};

RPZToyMetadata AssetsDatabase::importAsset(const RPZAssetImportPackage &package) {
    
    QMutexLocker l(&this->_m_withAssetsElems);

    if(package.isEmpty()) {
        qDebug() << "Asset : received empty import package !";
        return RPZToyMetadata();
    }
    
    RPZAssetHash asset_id = package["_id"].toString();

    //check content
    auto fileBytes = package["_fileContent"].toByteArray();
    if(!fileBytes.size()) {
        qDebug() << "Asset : requested" << asset_id << "cannot be found by server";
        return RPZToyMetadata();
    }

    //asset
    auto asset = QJsonDocument::fromVariant(package["_meta"]).object();

    //save file
    auto fileAsRawData = QByteArray::fromBase64(fileBytes);
    auto destFileExt = asset["ext"].toString();
    auto destFileName = asset["name"].toString();
    auto destUrl = this->_moveFileToDbFolder(fileAsRawData, destFileExt, asset_id);

    //copy into db
    auto parent = this->_staticElements[DownloadedContainer];
    auto metadata = this->_addAssetToDb(asset_id, destUrl, parent, destFileName);

    //add to tree
    auto element = new AssetsDatabaseElement(metadata);
    this->_withAssetsElems.insert(asset_id, element);

    auto destUrlPath = destUrl.toString();
    qDebug() << "Assets :" << destFileName << "imported";
    return metadata;
}

RPZAssetImportPackage AssetsDatabase::prepareAssetPackage(const RPZAssetHash &id) {
    
    //json obj
    RPZAssetImportPackage package;
    package["_id"] = id;

    //determine id existance by fetching the file path
    auto pathToFile = this->getFilePathToAsset(id);
    if(pathToFile.isNull()) {
        qDebug() << "Asset : requested" << id << "not found";
        return package;
    }

    //append file as base64
    QFile assetFile(pathToFile);
    assetFile.open(QFile::ReadOnly);

        auto fileContent = assetFile.readAll().toBase64();
        package["_fileContent"] = QString(fileContent);

    assetFile.close();

    package["_meta"] = this->assets()[id].toObject();
    return package;
}

QString AssetsDatabase::getFilePathToAsset(const RPZAssetHash &id) {
    auto db_assets = this->assets();
    if(!db_assets.contains(id)) return NULL;

    auto assetJSON = db_assets[id].toObject();
    auto fileExtension =  assetJSON["ext"].toString();

    return getFilePathToAsset(id, fileExtension);
}

QString AssetsDatabase::getFilePathToAsset(AssetsDatabaseElement* asset) {
    return this->getFilePathToAsset(asset->id());
}

QString AssetsDatabase::getFilePathToAsset(const RPZAssetHash &id, const QString &ext) {
	return QString("%1/%2.%3")
					.arg(AppContext::getAssetsFolderLocation())
					.arg(id)
					.arg(ext);
}

///
///
///

QJsonObject AssetsDatabase::paths() {
    return this->_db["paths"].toObject();
}

QJsonObject AssetsDatabase::assets() {
    return assets(this->_db);
}

QJsonObject AssetsDatabase::assets(QJsonDocument &doc) {
    return doc["assets"].toObject();
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

QHash<RPZAssetPath, AssetsDatabaseElement*> AssetsDatabase::_generateFolderTreeFromDb() {
    
    //sort the keys
    auto paths = this->paths().keys();
    paths.sort();

    //to be created items
    QHash<RPZAssetPath, AssetsDatabaseElement*> containersToFill;

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

AssetsDatabaseElement* AssetsDatabase::_recursiveElementCreator(AssetsDatabaseElement* parent, QList<QString> &pathChunks) {
    
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

RPZToyMetadata AssetsDatabase::getAssetMetadata(const RPZAssetHash &id) {
    QMutexLocker l(&this->_m_withAssetsElems);
    auto ptr = this->_withAssetsElems[id];
    return ptr ? ptr->toyMetadata() : RPZToyMetadata();
}

const QList<RPZAssetHash> AssetsDatabase::getStoredAssetsIds() const {
    QMutexLocker l(&this->_m_withAssetsElems);
    return this->_withAssetsElems.keys();
}

void AssetsDatabase::_generateItemsFromDb(const QHash<RPZAssetPath, AssetsDatabaseElement*> &pathsToFillWithItems) {
    
    auto db_paths = this->paths();
    auto assets_db = this->assets();

    QMutexLocker l(&this->_m_withAssetsElems);

    //create items for each end-containers
    for (auto i = pathsToFillWithItems.constBegin(); i != pathsToFillWithItems.constEnd(); ++i) {
        
        //define
        auto path = i.key();
        auto lastContainer = i.value();

        //preapre for search
        auto items_ids = db_paths[path].toArray();
        
        //find items in db and create them
        for(auto id : items_ids) {
            
            auto idStr = id.toString();

            //if ID doesnt exist
            if(!assets_db.contains(idStr)) {
                qDebug() << "Assets : cannot insert an item as its ID is not found in the assets DB";
                continue;
            }

            //create
            auto asset = assets_db[idStr].toObject();

            auto metadata = RPZToyMetadata(
                lastContainer,
                AssetsDatabaseElement::toAtomType(lastContainer->insertType()),
                idStr,
                asset["name"].toString(),
                getFilePathToAsset(idStr, asset["ext"].toString()),
                JSONSerializer::toQSize(asset["shape"].toArray()),
                JSONSerializer::pointFromDoublePair(asset["center"].toArray())
            );

            auto elem = new AssetsDatabaseElement(metadata);

            this->_withAssetsElems.insert(idStr, elem);

        }
    }

}


///
///
///

RPZAssetHash AssetsDatabase::_getFileSignatureFromFileUri(const QUrl &url) {
    
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
        RPZAssetHash signature = QString::fromUtf8(
            QCryptographicHash::hash(
                sourceFile.readAll(), 
                QCryptographicHash::Keccak_224
            ).toHex()
        );

    sourceFile.close();

    return signature;
}

QUrl AssetsDatabase::_moveFileToDbFolder(const QByteArray &data, const QString &fileExt, const RPZAssetHash &id) {
    
    //turn encoded file from JSON into file
    auto dest = getFilePathToAsset(id, fileExt);

    //write file
    QFile assetFile(dest);
    auto isOpen = assetFile.open(QIODevice::WriteOnly);
        auto written = assetFile.write(data);
    assetFile.close();
    
    //dummy
    return QUrl(dest);
}

bool AssetsDatabase::_moveFileToDbFolder(const QUrl &url, const RPZAssetHash &id) {

    //dest file suffix
    QFileInfo fInfo(url.fileName());
    auto destFileExt = fInfo.suffix();
    auto destFileName = fInfo.baseName();
    
    //file as raw data
    QFile assetFile(url.toLocalFile());
    assetFile.open(QFile::ReadOnly);
        auto data = assetFile.readAll();
    assetFile.close();

    //
    auto destUrl = this->_moveFileToDbFolder(data, destFileExt, id);
    return !destUrl.isEmpty();
}

RPZToyMetadata AssetsDatabase::_addAssetToDb(
        const RPZAssetHash &id, 
        const QUrl &url, 
        AssetsDatabaseElement* parent, 
        const QString &forcedName
    ) {

    //prepare
    auto db_paths = this->paths();
    auto obj = this->_db.object();
    auto folderParentPath = parent->path();

    //extract path from URL
    QString pathToAsset = url.isLocalFile() ? url.toLocalFile() : url.toString();
    QFileInfo fInfo(url.fileName());

    //1.save new asset
    auto assets = this->assets();

        //define new asset item
        QJsonObject newAsset;
        newAsset["ext"] = fInfo.suffix();

        auto assetName = forcedName.isEmpty() ? fInfo.baseName() : forcedName;
        newAsset["name"] = assetName;

        auto assetSizeAndCenter = _defineSizeAndCenterToDbAsset(pathToAsset, newAsset);

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
    qDebug() << "Assets :" << fInfo.filePath() << "inserted !";
    
    auto metadata = RPZToyMetadata(
        parent,
        AssetsDatabaseElement::toAtomType(parent->insertType()),
        id,
        assetName,
        pathToAsset,
        assetSizeAndCenter.size,
        assetSizeAndCenter.center
    );

    return metadata;
}

bool AssetsDatabase::insertAsset(const QUrl &url, AssetsDatabaseElement* parent) {

    QMutexLocker l(&this->_m_withAssetsElems);

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
    auto metadata = this->_addAssetToDb(fileSignature, url, parent);

    //add element
    auto element = new AssetsDatabaseElement(metadata);
    this->_withAssetsElems.insert(fileSignature, element);

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

QString AssetsDatabase::_generateNonExistingPath(AssetsDatabaseElement* parent, const QString &prefix) {
    
    QString generatedPath = "";
    RPZAssetPath destPath = parent->path();
    
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

void AssetsDatabase::_renameItem(const QString &name, AssetsDatabaseElement* target) {
    
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

    emit assetRenamed(target->id(), name);
}

void AssetsDatabase::_renameFolder(const QString &name, AssetsDatabaseElement* target) {

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

bool AssetsDatabase::rename(QString &name, AssetsDatabaseElement* target) { 

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
    auto ext = asset["ext"].toString();
    auto fp = AssetsDatabase::getFilePathToAsset(id, ext);

    QFile fileToRemove(fp);

    //remove stored file
    if(fileToRemove.exists()) {
        fileToRemove.remove();
    }
    
}

QSet<RPZAssetPath> AssetsDatabase::_getPathsToAlterFromList(const QList<AssetsDatabaseElement*> &elemsToAlter) {
    
    auto out = QSet<RPZAssetPath>();
    
    for(auto &elem : elemsToAlter) {
        if(elem->isContainer()) {
            out.insert(elem->path());
        }
    }

    return out;
}

QHash<RPZAssetPath, QSet<RPZAssetHash>> AssetsDatabase::_getAssetsToAlterFromList(const QList<AssetsDatabaseElement*> &elemsToAlter) {
    auto out = QHash<RPZAssetPath, QSet<RPZAssetHash>>();
    
    for(auto &elem : elemsToAlter) {
        if(elem->isIdentifiable()) {
            out[elem->path()].insert(elem->id());
        }
    }

    return out;
}

void AssetsDatabase::_augmentAssetsHashWithMissingDescendents(QHash<RPZAssetPath, QSet<RPZAssetHash>> &hashToAugment, const QSet<RPZAssetPath> &morePathsToDelete) {
    
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

QList<RPZAssetHash> AssetsDatabase::_removeIdsFromPaths(QJsonObject &db_paths, const QHash<RPZAssetPath, QSet<RPZAssetHash>> &idsToRemoveByPath) {
    
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
        auto augmentedList = QSet<RPZAssetPath> { pathToMove };
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
    obj["paths"] = db_paths;
    this->_updateDbFile(obj);

    //update model
    for(auto item : higherElems) {
        target->appendChild(item);
    }

    return true;
}

QSet<RPZAssetPath> AssetsDatabase::_augmentPathsSetWithMissingDescendents(QSet<RPZAssetPath> &setToAugment) {
        
    //remove already planned alterations
    QSet<RPZAssetPath> inheritedPathAlterations;
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

QHash<JSONDatabaseVersion, JSONDatabaseUpdateHandler> AssetsDatabase::_getUpdateHandlers() {
    
    auto out = QHash<JSONDatabaseVersion, JSONDatabaseUpdateHandler>();

    //to v5
    out.insert(
        5,
        [&](QJsonDocument &doc) {
            
            auto assets = AssetsDatabase::assets(doc);

            //iterate assets
            for(auto i = assets.begin(); i != assets.end(); i++) {
                
                auto assetId = i.key();
                auto assetDataRef = i.value();

                //get the filepath to the asset
                auto fpAsset = getFilePathToAsset(
                    i.key(),
                    assetDataRef.toObject()["ext"].toString()
                );

                //add shape + center data
                auto copy = assetDataRef.toObject();
                _defineSizeAndCenterToDbAsset(fpAsset, copy);
                assetDataRef = copy;

            }

            //update doc
            auto copy = doc.object();
            copy["assets"] = assets;
            doc.setObject(copy);

        }
    );

    return out;

}

SizeAndCenter AssetsDatabase::_defineSizeAndCenterToDbAsset(const QString &assetFilePath, QJsonObject &toUpdate) {
    
    SizeAndCenter out;
    QImage image(assetFilePath);
    
    out.size = image.size();
    out.center = QPointF (
        (qreal)out.size.width() / 2,
        (qreal)out.size.height() / 2
    );

    toUpdate["shape"] = JSONSerializer::fromQSize(out.size);
    toUpdate["center"] = JSONSerializer::pointToDoublePairJSON(out.center);

    return out;
}