// #pragma once

// #include <algorithm>

// #include <QString>
// #include <QVariantHash>
// #include <QCryptographicHash>
// #include <QMutexLocker>

// #include "src/shared/models/toy/RPZAsset.hpp"
// #include "src/shared/models/toy/RPZAssetImportPackage.hpp"
// #include "src/shared/database/_base/JSONDatabase.h"

// #include "src/helpers/_appContext.h"


// typedef QString RPZFolderPath; //internal DB arborescence path (only containers)

// class AssetsDatabase : public JSONDatabase {
    
//     Q_OBJECT

//     public:
//         enum class StorageContainer {
//             NPC = 301,
//             FloorBrush = 401,
//             Object = 501,
//             Downloaded = 601,
//             Background = 701
//         };

//         //singleton
//         static AssetsDatabase* get();

//         //CRUD methods
//         bool createFolder(AssetsTreeViewItem* parent);
//         bool insertAsset(const QUrl &url, AssetsTreeViewItem* parent);
//         bool rename(QString &name, AssetsTreeViewItem* target);
//         bool removeItems(const QList<AssetsTreeViewItem*> elemsToRemove);
//         bool moveItemsToContainer(const QList<AssetsTreeViewItem*> selectedItemsToMove, AssetsTreeViewItem* target);

//         //
//         RPZToyMetadata getAssetMetadata(const RPZAssetHash &id);
//         const QSet<RPZAssetHash> getStoredAssetsIds() const;

//         //network import/export
//         void importAsset(RPZAssetImportPackage &package);
//         const RPZAssetImportPackage prepareAssetPackage(const RPZAssetHash &id) const;

//     signals:
//         void assetRenamed(const RPZAssetHash &id, const QString &newName);

//     protected:
//         const int apiVersion() override;
//         void _removeDatabaseLinkedFiles() override;

//         QMap<RPZFolderPath, QList<RPZAssetHash>> _paths;
//         QHash<RPZAssetHash, RPZAsset> _assets;

//         const RPZAsset* asset(const RPZAssetHash &hash) const; 
//         void addAsset(const RPZAsset &asset, const RPZFolderPath &internalPathToAddTo);
    
//     private:
//         //
//         const QString _path(const StorageContainer &targetContainer) const;

//         //singleton
//         AssetsDatabase();
//         AssetsDatabase(const QJsonObject &doc);
//         static inline AssetsDatabase* _singleton = nullptr;

//         //updates handlers
//         QHash<JSONDatabase::Version, JSONDatabase::UpdateHandler> _getUpdateHandlers() override;
//         JSONDatabase::Model _getDatabaseModel() override;
//         void _setupLocalData() override;


//         //createFolder() helpers
//         QString _generateNonExistingPath(AssetsTreeViewItem* parent, const QString &prefix);

//         //rename() helpers
//         void _renameItem(const QString &name, AssetsTreeViewItem* target);
//         void _renameFolder(const QString &name, AssetsTreeViewItem* target);

//         //insertAsset() helpers
//         bool _moveFileToDbFolder(const QUrl &url, const RPZAssetHash &id);
//         QUrl _moveFileToDbFolder(const QByteArray &data, const QString &fileExt, const RPZAssetHash &id);
//         RPZToyMetadata _addAssetToDb(const RPZAssetHash &id, const QUrl &url, AssetsTreeViewItem* parent, const QString &forcedName = QString()); //returns asset metadata

//         //removeItems() helpers
//         QSet<RPZFolderPath> _getPathsToAlterFromList(const QList<AssetsTreeViewItem*> &elemsToAlter);
//         QHash<RPZFolderPath, QSet<RPZAssetHash>> _getAssetsToAlterFromList(const QList<AssetsTreeViewItem*> &elemsToAlter);
//         QSet<RPZFolderPath> _augmentPathsSetWithMissingDescendents(QSet<RPZFolderPath> &setToAugment);
//         void _augmentAssetsHashWithMissingDescendents(QHash<RPZFolderPath, QSet<RPZAssetHash>> &hashToAugment, const QSet<RPZFolderPath> &morePathsToDelete);
//         QList<RPZAssetHash> _removeIdsFromPaths(QJsonObject &db_paths, const QHash<RPZFolderPath, QSet<RPZAssetHash>> &idsToRemoveByPath); //returns removed ids
//         void _removeAssetsFromDb(QJsonObject &db_assets, const QList<RPZAssetHash> &assetIdsToRemove);
//         void _removeAssetFile(const RPZAssetHash &id, const QJsonObject &asset);

//         ////////////////////////////////////
//         // INITIAL Tree Injection helpers //
//         ////////////////////////////////////

//         QHash<AssetsTreeViewItem::Type, AssetsTreeViewItem*> _staticElements;     
//         void _injectStaticStructure();
        
//         void _injectDbStructure();              
            
//             //returns last elem by path created
//             QHash<RPZFolderPath, AssetsTreeViewItem*> _generateFolderTreeFromDb();

//             //iterate through paths chunks and create missing folders at each pass, returns last folder found/created
//             AssetsTreeViewItem* _recursiveElementCreator(AssetsTreeViewItem* parent, QList<QString> &pathChunks); 

//             //from definitive paths, fetch items from db and generate elements
//             void _generateItemsFromDb(const QHash<RPZFolderPath, AssetsTreeViewItem*> &pathsToFillWithItems);
// };








// #include "AssetsDatabase.h"

// AssetsDatabase::AssetsDatabase(const QJsonObject &doc) : JSONDatabase(doc) { }
// AssetsDatabase::AssetsDatabase() : JSONDatabase(AppContext::getAssetsFileCoordinatorLocation()) {};

// AssetsDatabase* AssetsDatabase::get() {
//     if(!_singleton) {
//         _singleton = new AssetsDatabase();
//     }
//     return _singleton;
// }

// const int AssetsDatabase::apiVersion() {
//     return 5;
// }

// JSONDatabase::Model AssetsDatabase::_getDatabaseModel() {
//     return {
//         { { QStringLiteral(u"paths"), JSONDatabase::EntityType::Object }, &this->_paths },
//         { { QStringLiteral(u"assets"), JSONDatabase::EntityType::Object }, &this->_assets }
//     };
// }


// void AssetsDatabase::_setupLocalData() {
    
//     //assets
//     for(auto &i : this->entityAsObject(QStringLiteral(u"assets"))) {
//         auto asset = RPZAsset(i.toVariant().toHash());
//         this->_assets.insert(asset.hash(), asset);
//     }

//     //paths
//     for(auto &i : this->entityAsObject(QStringLiteral(u"paths"))) {
//         auto paths = i.toObject();
//         for(auto &path : paths.keys()) {
//             auto assetHashes = paths.value(path);
//             this->_paths.insert(path, assetHashes.toVariant().toStringList());
//         }
//     }

// }

// void AssetsDatabase::_removeDatabaseLinkedFiles() {
//     QDir(AppContext::getAssetsFolderLocation()).removeRecursively();
// }

// const RPZAsset* AssetsDatabase::asset(const RPZAssetHash &hash) const {
//     if(!this->_assets.contains(hash)) {
//         qDebug() << "Assets: cannot find asset !";
//         return nullptr;
//     }
//     return &this->_assets[hash];
// }

// const QString AssetsDatabase::_path(const StorageContainer &targetContainer) const {
//     auto path = QStringLiteral(u"/{%1}").arg(QString::number((int)targetContainer));
//     return path;
// }

// void AssetsDatabase::addAsset(const RPZAsset &asset, const RPZFolderPath &internalPathToAddTo) {
    
//     //asset
//     auto hash = asset.hash();
//     this->_assets.insert(hash, asset);
    
//     //path
//     auto &assetsOfPath = this->_paths[internalPathToAddTo];
//     assetsOfPath.append(hash);

// }

// const RPZAssetImportPackage AssetsDatabase::prepareAssetPackage(const RPZAssetHash &id) const {
    
//     auto asset = this->asset(id);
//     if(!asset) return;

//     RPZAssetImportPackage package(*asset);
//     return package;

// }

// void AssetsDatabase::importAsset(RPZAssetImportPackage &package) {
    
//     auto success = package.tryIntegratePackage();
//     if(!success) return;

//     this->addAsset(
//         package, 
//         this->_path(StorageContainer::Downloaded)
//     );

// }


// ///
// ///
// ///

// void AssetsDatabase::_injectStaticStructure() {

//     for(auto &staticType : AssetsTreeViewItem::staticContainerTypes()) {

//         auto staticFolder = new AssetsTreeViewItem(AssetsTreeViewItem::typeDescription(staticType), this, staticType);
//         this->_staticElements.insert(staticType, staticFolder);

//         if(staticType == InternalContainer) {
//             for(auto &type : AssetsTreeViewItem::internalItemTypes()) {
//                 auto internalItem = new AssetsTreeViewItem(AssetsTreeViewItem::typeDescription(type), staticFolder, type);
//             }
//         }

//     }

// }


// void AssetsDatabase::_injectDbStructure() {

//     //get containers to fill, not inbetween folders
//     auto containersToFill = this->_generateFolderTreeFromDb();

//     //then fill with items
//     this->_generateItemsFromDb(containersToFill);

// }

// QHash<RPZFolderPath, AssetsTreeViewItem*> AssetsDatabase::_generateFolderTreeFromDb() {
    
//     //sort the keys
//     auto paths = this->paths().keys();
//     paths.sort();

//     //to be created items
//     QHash<RPZFolderPath, AssetsTreeViewItem*> containersToFill;

//     //create folders arbo
//     for(auto &path : paths) {
        
//         //split the path
//         auto split = AssetsTreeViewItem::pathAsList(path);
        
//         //make sure first split is a type
//         auto staticCType = AssetsTreeViewItem::pathChunktoType(split.takeFirst());

//         //get element from static source
//         if(!this->_staticElements.contains(staticCType)) {
//             qDebug() << "Assets : ignoring path, as the static container it points to doesnt exist";
//             continue;
//         }
//         auto staticContainerElem = this->_staticElements[staticCType];

//         //create path
//         auto lastContainer = this->_recursiveElementCreator(staticContainerElem, split);
        
//         //append to list
//         containersToFill.insert(path, lastContainer);
//     }

//     //returns
//     return containersToFill;
// }

// AssetsTreeViewItem* AssetsDatabase::_recursiveElementCreator(AssetsTreeViewItem* parent, QList<QString> &pathChunks) {
    
//     //if no more folders in path to create...
//     if(!pathChunks.count()) return parent;

//     //take first part
//     auto part = pathChunks.takeFirst();

//     //search if already exist
//     AssetsTreeViewItem* found = nullptr;
//     for(auto container : parent->childrenContainers()) {
//         if(container->displayName() == part) {
//             found = container;
//             break;
//         }
//     }

//     //if not found, create it
//     if(!found) {
//         found = new AssetsTreeViewItem(part, parent);
//     }

//     //iterate through...
//     return this->_recursiveElementCreator(found, pathChunks);

// }

// RPZToyMetadata AssetsDatabase::getAssetMetadata(const RPZAssetHash &id) {
//     QMutexLocker l(&this->_m_withAssetsElems);
//     auto ptr = this->_withAssetsElems.value(id);
//     return ptr ? ptr->toyMetadata() : RPZToyMetadata();
// }

// const QSet<RPZAssetHash> AssetsDatabase::getStoredAssetsIds() const {
//     QMutexLocker l(&this->_m_withAssetsElems);
//     auto out = this->_withAssetsElems.keys().toSet();
//     return out;
// }

// void AssetsDatabase::_generateItemsFromDb(const QHash<RPZFolderPath, AssetsTreeViewItem*> &pathsToFillWithItems) {
    
//     auto db_paths = this->paths();
//     auto assets_db = this->assets();

//     QMutexLocker l(&this->_m_withAssetsElems);

//     //create items for each end-containers
//     for (auto i = pathsToFillWithItems.constBegin(); i != pathsToFillWithItems.constEnd(); ++i) {
        
//         //define
//         auto path = i.key();
//         auto lastContainer = i.value();

//         //preapre for search
//         auto items_ids = db_paths[path].toArray();
        
//         //find items in db and create them
//         for(auto id : items_ids) {
            
//             auto idStr = id.toString();

//             //if ID doesnt exist
//             if(!assets_db.contains(idStr)) {
//                 qDebug() << "Assets : cannot insert an item as its ID is not found in the assets DB";
//                 continue;
//             }

//             //create
//             auto asset = assets_db[idStr].toObject();

//             auto metadata = RPZToyMetadata(
//                 lastContainer,
//                 AssetsTreeViewItem::toAtomType(lastContainer->insertType()),
//                 idStr,
//                 asset[QStringLiteral(u"name")].toString(),
//                 getFilePathToAsset(idStr, asset[QStringLiteral(u"ext")].toString()),
//                 JSONSerializer::toQSize(asset[QStringLiteral(u"shape")].toArray()),
//                 JSONSerializer::pointFromDoublePair(asset[QStringLiteral(u"center")].toArray())
//             );

//             auto elem = new AssetsTreeViewItem(metadata);

//             this->_trackAssetByElem(idStr, elem);

//         }
//     }

// }


// ///
// ///
// ///

// QUrl AssetsDatabase::_moveFileToDbFolder(const QByteArray &data, const QString &fileExt, const RPZAssetHash &id) {
    
//     //turn encoded file from JSON into file
//     auto dest = getFilePathToAsset(id, fileExt);

//     //write file
//     QFile assetFile(dest);
//     auto isOpen = assetFile.open(QIODevice::WriteOnly);
//         auto written = assetFile.write(data);
//     assetFile.close();
    
//     //dummy
//     return QUrl(dest);
// }

// bool AssetsDatabase::_moveFileToDbFolder(const QUrl &url, const RPZAssetHash &id) {

//     //dest file suffix
//     QFileInfo fInfo(url.fileName());
//     auto destFileExt = fInfo.suffix();
//     auto destFileName = fInfo.baseName();
    
//     //file as raw data
//     QFile assetFile(url.toLocalFile());
//     assetFile.open(QFile::ReadOnly);
//         auto data = assetFile.readAll();
//     assetFile.close();

//     //
//     auto destUrl = this->_moveFileToDbFolder(data, destFileExt, id);
//     return !destUrl.isEmpty();
// }

// RPZToyMetadata AssetsDatabase::_addAssetToDb(
//         const RPZAssetHash &id, 
//         const QUrl &url, 
//         AssetsTreeViewItem* parent, 
//         const QString &forcedName
//     ) {

//     //prepare
//     auto db_paths = this->paths();
//     auto obj = this->_db.object();
//     auto folderParentPath = parent->path();

//     //extract path from URL
//     QString pathToAsset = url.isLocalFile() ? url.toLocalFile() : url.toString();
//     QFileInfo fInfo(url.fileName());

//     //1.save new asset
//     auto assets = this->assets();

//         //define new asset item
//         QJsonObject newAsset;
//         newAsset[QStringLiteral(u"ext")] = fInfo.suffix();

//         auto assetName = forcedName.isEmpty() ? fInfo.baseName() : forcedName;
//         newAsset[QStringLiteral(u"name")] = assetName;

//         auto assetSizeAndCenter = _defineSizeAndCenterToDbAsset(pathToAsset, newAsset);

//     assets.insert(id, newAsset);
//     obj[QStringLiteral(u"assets")] = assets;
        
//     //check path existance, create it if non existant
//     QJsonArray objsInPath = db_paths.contains(folderParentPath) ? db_paths[folderParentPath].toArray() : QJsonArray();
//     if(!objsInPath.contains(id)) {

//         //2. update path status
//         objsInPath.append(id);
//         db_paths[folderParentPath] = objsInPath;
//         obj[QStringLiteral(u"paths")] = db_paths;
//     }


//     //update db file
//     this->_updateDbFile(obj);
//     qDebug() << "Assets :" << fInfo.filePath() << "inserted !";
    
//     auto metadata = RPZToyMetadata(
//         parent,
//         AssetsTreeViewItem::toAtomType(parent->insertType()),
//         id,
//         assetName,
//         pathToAsset,
//         assetSizeAndCenter.size,
//         assetSizeAndCenter.center
//     );

//     return metadata;
// }

// bool AssetsDatabase::insertAsset(const QUrl &url, AssetsTreeViewItem* parent) {

//     QMutexLocker l(&this->_m_withAssetsElems);

//     //get the corresponding signature
//     auto fileSignature = this->_getFileSignatureFromFileUri(url);
//     if(fileSignature.isNull()) return false;

//     // comparaison from db
//     auto fileSignatures = this->assets().keys();
//     if(fileSignatures.contains(fileSignature)) {
//         qDebug() << "Assets : will not insert, file signature has been found in db !";
//         return false;
//     }

//     //copy file
//     if(!this->_moveFileToDbFolder(url, fileSignature)) return false;

//     //insert into db
//     auto metadata = this->_addAssetToDb(fileSignature, url, parent);

//     //add element
//     auto element = new AssetsTreeViewItem(metadata);
//     this->_trackAssetByElem(fileSignature, element);

//     return true;
// }


// bool AssetsDatabase::createFolder(AssetsTreeViewItem* parent) {
    
//     //data template
//     auto generatedPath = this->_generateNonExistingPath(parent, "Dossier");

//     //save
//     auto obj = this->_db.object();
//     auto db_paths = this->paths();
//     db_paths[generatedPath] = QJsonArray();
//     obj[QStringLiteral(u"paths")] = db_paths;
//     this->_updateDbFile(obj);

//     //create elem
//     auto generatedName = AssetsTreeViewItem::pathAsList(generatedPath).takeLast();
//     auto folder = new AssetsTreeViewItem(generatedName, parent);

//     return true;
// }

// QString AssetsDatabase::_generateNonExistingPath(AssetsTreeViewItem* parent, const QString &prefix) {
    
//     QString generatedPath = "";
//     RPZFolderPath destPath = parent->path();
    
//     //prepare path generation
//     auto generateNewPath = [destPath, prefix]() {
//         auto newPath = destPath;
//         newPath += "/" + prefix + "_";
//         newPath += QString::number(QDateTime().currentMSecsSinceEpoch());
//         return newPath;
//     }; 

//     //generate non existing path in db
//     auto db_paths = this->paths();
//     do {
//         generatedPath = generateNewPath();
//     } while(db_paths.contains(generatedPath));

//     //returns
//     return generatedPath;

// }

// void AssetsDatabase::_renameItem(const QString &name, AssetsTreeViewItem* target) {
    
//     //data template
//     auto obj = this->_db.object();
    
//     //prepare
//     auto db_assets = this->assets();
//     auto targetId = target->id();

//     //find current id in db
//     auto idExists = db_assets.contains(targetId);
//     if(!idExists) {
//         qDebug() << "Assets : cannot rename since the ID of the target cannot be found in the DB";
//         return;
//     }

//     //save with new name
//     auto asset = db_assets[targetId].toObject();
//     asset[QStringLiteral(u"name")] = name;
//     db_assets[targetId] = asset;
//     obj[QStringLiteral(u"assets")] = db_assets;

//     //update db
//     this->_updateDbFile(obj);

//     emit assetRenamed(target->id(), name);
// }

// void AssetsDatabase::_renameFolder(const QString &name, AssetsTreeViewItem* target) {

//     //data template
//     auto obj = this->_db.object();

//     //prepare
//     auto currentPath = target->path();
//     auto db_paths = this->paths();

//     //find current path in db
//     auto pathExists = db_paths.contains(currentPath);
//     if(!pathExists) {
//         qDebug() << "Assets : cannot rename since the actual path to rename is not in the DB";
//         return;
//     }

//     //update path
//     auto pathAsList = AssetsTreeViewItem::pathAsList(currentPath);
//     pathAsList.takeLast();
//     pathAsList.append(name);
//     auto updatedPath = "/" + pathAsList.join("/");

//     //search for paths beeing part of the old path, and replace their value
//     for(auto &path : db_paths.keys()) {
        
//         //skip if not targeted
//         if(!path.startsWith(currentPath)) continue; 

//         //rebuild path
//         auto rebuiltPath = path;
//         rebuiltPath.replace(currentPath, updatedPath);

//         //replace old path
//         auto tempPathContent = db_paths[path].toArray();
//         db_paths.remove(path);
//         db_paths[rebuiltPath] = tempPathContent;
//     }

//     obj[QStringLiteral(u"paths")] = db_paths;

//     //update db
//     this->_updateDbFile(obj);
// }

// bool AssetsDatabase::rename(QString &name, AssetsTreeViewItem* target) { 

//     //check if acceptable name change
//     if(!target->isAcceptableNameChange(name)) return false;

//     //rename for items
//     if(target->isIdentifiable()) this->_renameItem(name, target);

//     //rename for folders
//     else if(target->type() == Folder) this->_renameFolder(name, target);

//     //should not happend
//     else return false;

//     //finally rename element
//     target->rename(name);
//     return true;
// }

// ////
// ///
// ///
// ////

// void AssetsDatabase::_removeAssetFile(const RPZAssetHash &id, const QJsonObject &asset) {
    
//     //prepare
//     auto ext = asset[QStringLiteral(u"ext")].toString();
//     auto fp = AssetsDatabase::getFilePathToAsset(id, ext);

//     QFile fileToRemove(fp);

//     //remove stored file
//     if(fileToRemove.exists()) {
//         fileToRemove.remove();
//     }
    
// }

// QSet<RPZFolderPath> AssetsDatabase::_getPathsToAlterFromList(const QList<AssetsTreeViewItem*> &elemsToAlter) {
    
//     auto out = QSet<RPZFolderPath>();
    
//     for(auto &elem : elemsToAlter) {
//         if(elem->isContainer()) {
//             out.insert(elem->path());
//         }
//     }

//     return out;
// }

// QHash<RPZFolderPath, QSet<RPZAssetHash>> AssetsDatabase::_getAssetsToAlterFromList(const QList<AssetsTreeViewItem*> &elemsToAlter) {
//     auto out = QHash<RPZFolderPath, QSet<RPZAssetHash>>();
    
//     for(auto &elem : elemsToAlter) {
//         if(elem->isIdentifiable()) {
//             out[elem->path()].insert(elem->id());
//         }
//     }

//     return out;
// }

// void AssetsDatabase::_augmentAssetsHashWithMissingDescendents(QHash<RPZFolderPath, QSet<RPZAssetHash>> &hashToAugment, const QSet<RPZFolderPath> &morePathsToDelete) {
    
//     auto db_paths = this->paths();

//     //additional items to alter from paths arrays
//     for(auto &path : morePathsToDelete) {

//         //if does not contains targeted path, skip
//         if(!db_paths.contains(path)) break;

//         //if it does, add it to the list
//         auto idsInPath = db_paths[path].toArray();
//         for(auto id : idsInPath) {
//             hashToAugment[path].insert(id.toString());
//         }
//     }

// }

// QList<RPZAssetHash> AssetsDatabase::_removeIdsFromPaths(QJsonObject &db_paths, const QHash<RPZFolderPath, QSet<RPZAssetHash>> &idsToRemoveByPath) {
    
//     QList<RPZAssetHash> ids;

//     //delete for each path
//     for (auto i = idsToRemoveByPath.constBegin(); i != idsToRemoveByPath.constEnd(); ++i) { 
        
//         //preapre
//         auto path = i.key();
//         auto idsToDelete = i.value();

//         ids.append(idsToDelete.toList());

//         //define ids left for the path
//         auto IdsInPath = db_paths[path].toArray();
//         auto idsLeftInPath = JSONDatabase::diff(
//             IdsInPath, 
//             idsToDelete
//         );

//         //if ids are still left, update the array
//         if(idsLeftInPath.count()) {
//             db_paths[path] = idsLeftInPath;
//         } 

//         // if no more ids, just delete the key
//         else {
//             db_paths.remove(path);
//         }
//     }

//     return ids;
// }

// void AssetsDatabase::_removeAssetsFromDb(QJsonObject &db_assets, const QList<RPZAssetHash> &assetIdsToRemove) {

//     QMutexLocker l(&this->_m_withAssetsElems);

//     //finally delete items
//     for(auto &id : assetIdsToRemove) {

//         auto asset = db_assets[id].toObject();

//         //remove
//         db_assets.remove(id);
        
//         //
//         this->_withAssetsElems.remove(id);

//         //remove stored file
//         this->_removeAssetFile(id, asset);
//     }
// }

// bool AssetsDatabase::removeItems(QList<AssetsTreeViewItem*> elemsToRemove) {

//     //prepare
//     auto db_paths = this->paths();
//     auto db_assets = this->assets();

//     //compartimentation
//     auto pathsToDelete = this->_getPathsToAlterFromList(elemsToRemove);
//     auto idsToRemoveByPath = this->_getAssetsToAlterFromList(elemsToRemove);

//     //augment list of paths to alter with children of paths already planned for alteration
//     auto morePathsToDelete = this->_augmentPathsSetWithMissingDescendents(pathsToDelete);

//     //since we augmented the paths, also impacts ids
//     this->_augmentAssetsHashWithMissingDescendents(idsToRemoveByPath, morePathsToDelete);
    
//     //remove ids from paths
//     auto removedIdsFromPaths = this->_removeIdsFromPaths(db_paths, idsToRemoveByPath);

//     //delete remaining paths, e.g. empty folders not auto-deleted by removeIdsFromPaths()
//     for(auto &path : pathsToDelete) {
//         if(db_paths.contains(path)) {
//             db_paths.remove(path);
//         }
//     }
    
//     //delete assets
//     this->_removeAssetsFromDb(db_assets, removedIdsFromPaths);

//     //save changes
//     auto obj = this->_db.object();
//     obj[QStringLiteral(u"assets")] = db_assets;
//     obj[QStringLiteral(u"paths")] = db_paths;
//     this->_updateDbFile(obj);

//     //sort items by path length (number of slashes); delete them first
//     AssetsTreeViewItem::sortByPathLengthDesc(elemsToRemove);
//     for(auto elem : elemsToRemove) {
//         //finally delete elems
//         delete elem;
//     }

//     return true;
// }



// bool AssetsDatabase::moveItemsToContainer(QList<AssetsTreeViewItem*> selectedItemsToMove, AssetsTreeViewItem* target) {
    
//     auto db_paths = this->paths();

//     //reduce targetedItems list to only the higher in order for each node
//     auto higherElems = AssetsTreeViewItem::filterTopMostOnly(selectedItemsToMove).toList();

//     //move assets
//     auto idsToMove = this->_getAssetsToAlterFromList(higherElems);
//     auto idsRemoved = this->_removeIdsFromPaths(db_paths, idsToMove);

//         //bind ids to target
//         auto targetPath = target->path();
//         auto target_db_path = db_paths[targetPath].toArray();
//         for(auto &id : idsRemoved) {
//             target_db_path.append(id);
//         }
//         db_paths[targetPath] = target_db_path;

    
//     //rename paths
//     auto pathsToMove = this->_getPathsToAlterFromList(higherElems);

//     auto parentPath = target->path();

//     for(auto &pathToMove : pathsToMove) {

//         //for each higher elem, augment with descendants
//         auto augmentedList = QSet<RPZFolderPath> { pathToMove };
//         this->_augmentPathsSetWithMissingDescendents(augmentedList);

//         //define new path
//         auto displayName = AssetsTreeViewItem::pathAsList(pathToMove).takeLast();
//         auto newPath = parentPath + "/" + displayName;

//         for(auto &path : augmentedList) {
            
//             //generate the dest path
//             auto newFullPath = path;
//             newFullPath.replace(pathToMove, newPath);

//             //get/create the dest path in db
//             auto dest = db_paths[newFullPath].toArray();

//             //if there are ids to transfert
//             auto content = db_paths[path].toArray();
//             if(content.count()) {
//                 for(auto id : content) {
//                     dest.append(id);
//                 }
//             }

//             //erase new path array by new one
//             db_paths[newFullPath] = dest;

//             //remove old path
//             db_paths.remove(path);
//         }
        
//     }

//     //update db
//     auto obj = this->_db.object();
//     obj[QStringLiteral(u"paths")] = db_paths;
//     this->_updateDbFile(obj);

//     //update model
//     for(auto item : higherElems) {
//         target->appendChild(item);
//     }

//     return true;
// }

// QSet<RPZFolderPath> AssetsDatabase::_augmentPathsSetWithMissingDescendents(QSet<RPZFolderPath> &setToAugment) {
        
//     //remove already planned alterations
//     QSet<RPZFolderPath> inheritedPathAlterations;
//     auto remaining_db_paths = this->paths().keys().toSet().subtract(setToAugment);

//     //check if a path to be remove is a part of a remaining path 
//     for(auto &remaining_path : remaining_db_paths) {
        
//         for(auto &pathToCompareTo : setToAugment) {
            
//             //if so, adds it to alteration list
//             if(remaining_path.startsWith(pathToCompareTo)) {
//                 inheritedPathAlterations.insert(remaining_path);
//             }

//         }
//     }

//     //fusion of sets
//     for(auto &path : inheritedPathAlterations) {
//         setToAugment.insert(path);
//     }

//     return inheritedPathAlterations;
// }

// QHash<JSONDatabase::Version, JSONDatabase::UpdateHandler> AssetsDatabase::_getUpdateHandlers() {
    
//     auto out = QHash<JSONDatabase::Version, JSONDatabase::UpdateHandler>();

//     //to v5
//     out.insert(
//         5,
//         [&](QJsonObject &doc) {
            
//             AssetsDatabase db(doc);

//             //iterate assets
//             QVariantHash compiled;
//             for(auto &asset : db._assets) {

//                 //try to read associated asset and define geometry
//                 asset.updateAssetGeometryData();
//                 compiled.insert(asset.hash(), asset);

//             }

//             //update json obj
//             updateFrom(
//                 doc,
//                 QStringLiteral(u"assets"),
//                 compiled
//             );

//         }
//     );

//     return out;

// }
