#include "AssetsDatabase.h"

AssetsDatabase::AssetsDatabase() { 
    this->_instanciateDb();
    this->_includeStaticComponents();
    this->_includeDbComponents();
};

const QString AssetsDatabase::defaultJsonDoc() {
    return "{\"version\":1,\"paths\":{},\"hashes\":[]}";
};
const QString AssetsDatabase::dbPath() {
    return QString::fromStdString(getAssetsFileCoordinatorLocation());
};

///
///
///

void AssetsDatabase::_includeDbComponents() {
    
    //data
    auto db_paths = this->_db["paths"].toObject();
    auto assets_db = this->_db["assets"].toObject();

    //sort the keys
    auto paths = db_paths.keys();
    paths.sort();

    //to be created items
    QHash<QString, AssetsDatabaseElement*> containersToFill;

    //create folders arbo
    for(auto &path : paths) {
        
        //split the path
        auto split = path.split("/", QString::SplitBehavior::SkipEmptyParts);
        
        //make sure first split is a type
        auto firstSplit = split.takeFirst();
        auto expected = firstSplit.startsWith("{") && firstSplit.endsWith("}");
        if(!expected) {
            qDebug() << "Assets : ignoring path, as its structure is not expected.";
            continue;
        }
        
        //type cast and get element type
        firstSplit.replace("{", "");
        firstSplit.replace("}", "");
        auto castOk = false;
        auto staticCType = (AssetsDatabaseElement::Type)firstSplit.toInt(&castOk);
        if(!castOk) {
            qDebug() << "Assets : ignoring path, as static container type was impossible to deduce";
            continue;
        }

        //get element from static hash
        if(!this->_staticElements.contains(staticCType)) {
            qDebug() << "Assets : ignoring path, as the static container it points to doesnt exist";
            continue;
        }
        auto staticContainerElem = this->_staticElements[staticCType];

        //create path
        auto lastContainer = this->_helperPathCreation(staticContainerElem, split);
        
        //append to list
        containersToFill.insert(path, lastContainer);
    }

    //create items for each end-containers
    QHash<QString, AssetsDatabaseElement*>::iterator i;
    for (i = containersToFill.begin(); i != containersToFill.end(); ++i) {
        
        //define
        auto path = i.key();
        auto lastContainer =  i.value();

        //preapre for search
        auto items_ids = db_paths[path].toArray();
        auto lastContainerInsertType = lastContainer->insertType();
        
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
            auto newElem = new AssetsDatabaseElement(assetName, lastContainer, lastContainerInsertType, idStr);
        }
    }

}

AssetsDatabaseElement* AssetsDatabase::_helperPathCreation(AssetsDatabaseElement* parent, QList<QString> paths) {
    
    //if no more folders in path to create...
    if(!paths.count()) return parent;

    //take first part
    auto part = paths.takeFirst();

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
    return this->_helperPathCreation(found, paths);

}

void AssetsDatabase::_includeStaticComponents() {

    auto staticModelInsert = [&](QString name, AssetsDatabaseElement::Type type) {
        auto i = new AssetsDatabaseElement(name, this, type);
        this->_staticElements.insert(type, i);
        return i;
    };

    /* /Internal... */
    auto interne = staticModelInsert("Interne", AssetsDatabaseElement::Type::InternalContainer);
            
        /* /Internal/Player... */
        auto player = new AssetsDatabaseElement("Joueur", interne, AssetsDatabaseElement::Type::Player);

        /* /Internal/Trigger... */
        auto trigger = new AssetsDatabaseElement("Evenement", interne, AssetsDatabaseElement::Type::Event);

    /* /NPC... */
    auto npc = staticModelInsert("PNJ", AssetsDatabaseElement::Type::NPC_Container);

    /* /MapAssets... */
    auto mapAssets = staticModelInsert("Assets de carte", AssetsDatabaseElement::Type::ObjectContainer);

    /* /Brushes... */
    auto brushes = staticModelInsert("Terrains", AssetsDatabaseElement::Type::FloorBrushContainer);

}


bool AssetsDatabase::insertAsset(QUrl &url, AssetsDatabaseElement* parent) {

    //check if db file exists, recreate it if necessary
    this->_checkFileExistance();

    //check scheme
    if(url.scheme() != "file") {
        qDebug() << "Assets : cannot insert, uri is not a file !";
        return false;
    }

    //prepare
    auto source = url.toString(QUrl::UrlFormattingOption::RemoveScheme).mid(3);
    auto sourceFile = QFile(source);
    if(!sourceFile.exists()) {
        qDebug() << "Assets : cannot insert, uri file does not exist !";
        return false;
    }

    //read hash..
    sourceFile.open(QFile::ReadOnly);
    auto hash = QString::fromUtf8(
        QCryptographicHash::hash(sourceFile.readAll(), QCryptographicHash::Keccak_224).toHex()
    );
    sourceFile.close();

        // comparaison from db
        auto db_hashes = this->_db["hashes"].toArray();
        if(db_hashes.contains(hash)) {
            qDebug() << "Assets : will not insert, file hash has been found in db !";
            return false;
        }

    //checksum and uuid
    auto quuid = QUuid::createUuid().toString();
    quuid = quuid.mid(1, quuid.length() - 2);

    //dest file
    auto destFolder = QString::fromStdString(getAssetsFolderLocation());
    auto fInfo = QFileInfo(url.fileName());
    auto destFileExt = fInfo.suffix();
    auto dest = destFolder + QDir::separator() + quuid + "." + destFileExt;

    //copy
    auto copyResult = QFile::copy(source, dest);
    if(!copyResult) {
        qDebug() << "Assets : cannot insert, issue while copying source file to dest !";
        return false;
    }

    //update db
    auto obj = this->_db.object();
        
        //update hashes list
        db_hashes.append(QJsonValue(hash));
        obj["hashes"] = db_hashes;

        //define new asset item
        auto newAsset = QJsonObject();
        newAsset["hash"] = hash;
        newAsset["ext"] = destFileExt;
        newAsset["name"] = fInfo.baseName();

            //save new asset
            auto assets = obj["assets"].toObject();
            assets.insert(quuid, newAsset);
            obj["assets"] = assets;
        
        //update path status
        auto folderParentPath = parent->path();
        auto db_paths = obj["paths"].toObject();

            //check path existance, create it if non existant
            QJsonArray objsInPath = db_paths.contains(folderParentPath) ? db_paths[folderParentPath].toArray() : QJsonArray();

            //append id to list
            if(!objsInPath.contains(quuid)) {
                objsInPath.append(quuid);
                db_paths[folderParentPath] = objsInPath;
                obj["paths"] = db_paths;
            }


    //update db file
    this->_updateDbFile(obj);
    qDebug() << "Assets : " << fInfo.filePath() << " inserted !";

    //add element
    auto element = new AssetsDatabaseElement(fInfo.fileName(), parent, parent->insertType(), quuid);
    return true;
}

QJsonArray AssetsDatabase::_diff(QJsonArray &target, QSet<QString> &toRemoveFromTarget) {
    auto output = QJsonArray();
    for(auto &e : target) {
        auto str = e.toString();
        if(!toRemoveFromTarget.contains(str)) {
            output.append(str);
        }
    }
    return output;
}

void AssetsDatabase::_prepareForAlteration(QList<AssetsDatabaseElement*> elemsToAlter) {
    
    //clear previous alteration cache
    this->_temp_pathsToAlter = QSet<QString>();
    this->_temp_idsToAlterByPath = QHash<QString, QSet<QString>>();
    this->_temp_db_paths = QJsonObject();

    //iterate through initial list
    for(auto &elem : elemsToAlter) {
        
        //for containers
        if(elem->isContainer()) {
            this->_temp_pathsToAlter.insert(elem->path());
        } 

        //for items
        else if(elem->isItem()) {
            this->_temp_idsToAlterByPath[elem->path()].insert(elem->id());
        }  

    }

    //augment list of paths to alter with children of paths already planned for alteration
    this->_temp_db_paths = this->_db["paths"].toObject();

        //remove already planned alterations
        QSet<QString> inheritedPathAlterations;
        auto remaining_db_paths = this->_temp_db_paths.keys().toSet().subtract(this->_temp_pathsToAlter);

        //check if a path to be remove is a part of a remaining path 
        for(auto &remaining_path : remaining_db_paths) {
            
            for(auto &pathToCompareTo : this->_temp_pathsToAlter) {
                
                //if so, adds it to alteration list
                if(remaining_path.startsWith(pathToCompareTo)) {
                    inheritedPathAlterations.insert(remaining_path);
                }

            }
        }

        //fusion of sets
        for(auto &path : inheritedPathAlterations) {
            this->_temp_pathsToAlter.insert(path);
        }
        

    //additional items to alter from paths arrays
    for(auto &path : this->_temp_pathsToAlter) {

        //if does not contains targeted path, skip
        if(!this->_temp_db_paths.contains(path)) break;

        //if it does, add it to the list
        for(auto &id : this->_temp_db_paths[path].toArray()) {
            this->_temp_idsToAlterByPath[path].insert(id.toString());
        }
    }

    //delete for each path
    QHash<QString, QSet<QString>>::iterator i;
    for (i = this->_temp_idsToAlterByPath.begin(); i != this->_temp_idsToAlterByPath.end(); ++i) { 
        
        //preapre
        auto path = i.key();
        auto idsToDelete = i.value();

        //define ids left for the path
        auto idsLeftInPath = _diff(
            this->_temp_db_paths[path].toArray(), 
            idsToDelete
        );

        //if ids are still left, update the array
        if(idsLeftInPath.count()) {
            this->_temp_db_paths[path] = idsLeftInPath;
        } 

        // if no more ids, just delete the key
        else {
            this->_temp_db_paths.remove(path);
        }
    }

    //delete remaining paths, e.g. empty folders
    for(auto &path : this->_temp_pathsToAlter) {
        if(this->_temp_db_paths.contains(path)) {
            this->_temp_db_paths.remove(path);
        }
    }


}

bool AssetsDatabase::removeItems(QList<AssetsDatabaseElement*> elemsToRemove) {

    //prepare temporary data
    this->_prepareForAlteration(elemsToRemove);
    
    //prepare to delete items
    auto db_assets = this->_db["assets"].toObject();
    QSet<QString> hashesToRemove;

    //finally delete items
    for(auto &idsList : this->_temp_idsToAlterByPath) {
        for(auto &id : idsList) {

            //append hash to remove
            auto asset = db_assets[id].toObject();
            auto hash = asset["hash"].toString();
            hashesToRemove.insert(hash);

            //remove
            db_assets.remove(id);

            //remove stored file
            auto fileName = id + "." + asset["ext"].toString();
            auto fileToRemove = QFile(fileName);
            if(fileToRemove.exists()) {
                fileToRemove.remove();
            }
        }
    }

    //update hashes array
    auto db_hashes = _diff(
        this->_db["hashes"].toArray(),
        hashesToRemove
    );

    //save changes
    auto obj = this->_db.object();
    obj["assets"] = db_assets;
    obj["hashes"] = db_hashes;
    obj["paths"] = this->_temp_db_paths;
    this->_updateDbFile(obj);

    //sort items by path length (number of slashes); delete them first
    struct {
        bool operator()(AssetsDatabaseElement* a, AssetsDatabaseElement* b) const {   
            return a->fullPath().count("/") > b->fullPath().count("/");
        }   
    } pathLength;
    std::sort(elemsToRemove.begin(), elemsToRemove.end(), pathLength);

    //finally delete elems
    for(auto elem : elemsToRemove) {
        delete elem;
    }

    return true;
}

bool AssetsDatabase::moveItems(QList<AssetsDatabaseElement*> targetedItems, AssetsDatabaseElement* target) {

    //prepare temporary data
    // this->_prepareForAlteration(targetedItems);

    //update dest path
    auto targetPath = target->path();

    //
    // this->_temp_pathsToAlter = QSet<QString>();
    // this->_temp_idsToAlterByPath = QHash<QString, QSet<QString>>();
    // this->_temp_db_paths = QJsonObject();
    
    


    //update model, reduce targetedItems list to only the higher in order for each node
    for(auto &item : _onlyHighestsElementsByPath(targetedItems)) {
        target->appendChild(item);
    }

    return true;
}

QSet<AssetsDatabaseElement*> AssetsDatabase::_onlyHighestsElementsByPath(QList<AssetsDatabaseElement*> elems) {

    QSet<AssetsDatabaseElement*> higher;
    while(elems.count()) {

        //take first
        if(!higher.count()) {
            higher.insert(elems.takeFirst());
            continue;
        }

        //compare
        auto toCompareTo = elems.takeFirst();
            auto compare_path = toCompareTo->fullPath();
            auto compare_length = compare_path.length();
        
        auto isForeigner = true;

        //iterate
        QSet<AssetsDatabaseElement*> obsoletePointers;
        for(auto &st : higher) {

            auto st_path = st->fullPath();
            auto st_length = st_path.length();

            auto outputArrayContainsBuffered = st_path.startsWith(compare_path);

            //if 
            if(isForeigner && (outputArrayContainsBuffered || compare_path.startsWith(st_path))) {
                isForeigner = false;
            }

            //must be deleted, compared path must be prefered
            if(outputArrayContainsBuffered && st_length > compare_length) {
                obsoletePointers.insert(st);
            }
        }

        //if no presence or better than a stored one
        if(isForeigner || obsoletePointers.count()) {
            higher.insert(toCompareTo);
        }

        //if obsolete pointers have been marked, remove them from the higherList and add the compared one to it
        if(obsoletePointers.count()) {
            higher.subtract(obsoletePointers);
        }

    }

    return higher;
}

bool AssetsDatabase::createFolder(AssetsDatabaseElement* parent) {
    
    //data template
    auto obj = this->_db.object();
    auto parentPath = parent->path();

    //prepare path generation
    QString generatedPath = "";
    auto generateNewPath = [parentPath]() {
        auto newPath = parentPath;
        newPath += "/Dossier_";
        newPath += QString::number(QDateTime().currentMSecsSinceEpoch());
        return newPath;
    }; 

    //generate non existing path in db
    auto db_paths = obj["paths"].toObject();
    do {
        generatedPath = generateNewPath();
    } while(db_paths.contains(generatedPath));

    //save
    db_paths[generatedPath] = QJsonArray();
    obj["paths"] = db_paths;
    this->_updateDbFile(obj);

    //create elem
    auto generatedName = generatedPath.split("/", QString::SplitBehavior::SkipEmptyParts).takeLast();
    auto folder = new AssetsDatabaseElement(generatedName, parent);

    return true;
}

bool AssetsDatabase::rename(QString name, AssetsDatabaseElement* target) { 

    //strip name from slashes and double quotes
    name.replace("\"", "");
    name.replace("/", "");

    //if empty name
    if(name.isEmpty()) return false;

    //if same name, no changes
    if(target->displayName() == name) return false;

    //data template
    auto obj = this->_db.object();

    //rename for items
    if(target->isItem()) {
        
        //prepare
        auto db_assets = obj["assets"].toObject();
        auto targetId = target->id();

        //find current id in db
        auto idExists = db_assets.contains(targetId);
        if(!idExists) {
            qDebug() << "Assets : cannot rename since the ID of the target cannot be found in the DB";
            return false;
        }

        //save with new name
        auto asset = db_assets[targetId].toObject();
        asset["name"] = name;
        db_assets[targetId] = asset;
        obj["assets"] = db_assets;

    } 
    
    //rename for folders
    else if(target->type() == Folder) {

        //prepare
        auto currentPath = target->path();
        auto db_paths = obj["paths"].toObject();

        //find current path in db
        auto pathExists = db_paths.contains(currentPath);
        if(!pathExists) {
            qDebug() << "Assets : cannot rename since the actual path to rename is not in the DB";
            return false;
        }

        //update path
        auto pathAsList = currentPath.split("/", QString::SplitBehavior::SkipEmptyParts);
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

    } else {

        //should not happend
        return false;

    }

    //update db and rename
    this->_updateDbFile(obj);
    target->rename(name);
    return true;
}