#include "AssetsTreeViewModel.h"

AssetsTreeViewModel::AssetsTreeViewModel(QObject *parent) : 
    QAbstractItemModel(parent), 
    _db(AssetsDatabase::get()) { };

AssetsDatabase* AssetsTreeViewModel::database() const {
    return this->_db;
}

QModelIndex AssetsTreeViewModel::_getDownloadableFolderIndex() {
    auto root = this->index(0, 0);
    return this->index(4, 0, root);
}

void AssetsTreeViewModel::integrateAsset(const RPZAssetImportPackage &package) {
    
    //get where exactly the new asset is supposed to be
    auto dlFolderIndex = this->_getDownloadableFolderIndex();
    auto dlFolder = AssetsDatabaseElement::fromIndex(dlFolderIndex);
    auto posToInsert = dlFolder->childCount();
    
    this->beginInsertRows(this->_getDownloadableFolderIndex(), posToInsert, posToInsert);
        auto metadata = this->_db->importAsset(package); 
    this->endInsertRows();

}

///////////////
/// HELPERS ///
///////////////

QPixmap AssetsTreeViewModel::getAssetIcon(AssetsDatabaseElement* target, QSize &sizeToApply) const {
    
    QPixmap toFind;

    //if selected elem is no item, skip
    if(!target->isItem()) return toFind;

    //search in cache
    auto idToSearch = target->id() + "_ico";
    auto isFound = QPixmapCache::find(idToSearch, &toFind);
    if(isFound) return toFind;

    //get asset from filepath
    auto fpToAssetFile = AssetsDatabase::get()->getFilePathToAsset(target);
    QPixmap pixmap(fpToAssetFile);

    //resize it to hint
    pixmap = pixmap.scaled(
        sizeToApply,
        Qt::AspectRatioMode::KeepAspectRatio 
    );

    //cache pixmap and return it
    QPixmapCache::insert(idToSearch, pixmap);
    return pixmap;
}

bool AssetsTreeViewModel::createFolder(QModelIndex &parentIndex) {
    this->beginInsertRows(parentIndex, 0, 0);
    
        auto data = AssetsDatabaseElement::fromIndex(parentIndex);
        auto result = this->_db->createFolder(data);

    this->endInsertRows();
    return result;
}

bool AssetsTreeViewModel::_bufferContainsIndexOrParent(const QModelIndex &index) {
    if(!index.isValid()) return false;
    if(this->_bufferedDraggedIndexes.contains(index)) return true;
    return this->_bufferContainsIndexOrParent(index.parent());
}

QModelIndexList AssetsTreeViewModel::_getTopMostIndexesFromDraggedIndexesBuffer() {
    
    QModelIndexList higher;
    auto elemsToFilter = this->_bufferedDraggedIndexes;

    while(elemsToFilter.count()) {

        auto c = elemsToFilter.takeFirst();

        //take first
        if(!higher.count()) {
            higher.append(c);
            continue;
        }

        if(!this->_bufferContainsIndexOrParent(c.parent())) {
            higher.append(c);
        }
        
    }

    return higher;
}

QPair<int, int> AssetsTreeViewModel::_anticipateInserts(const QModelIndexList &tbi) {
    
    int insertAtBegin = 0;
    int insertAtEnd = 0;
    
    for(auto &index : tbi) {
        auto elem = AssetsDatabaseElement::fromIndex(index);
        if(elem->type() == AssetsDatabaseElement::Type::Folder) insertAtBegin++;
        else insertAtEnd++;
    }

    return QPair<int, int>(insertAtBegin, insertAtEnd);
}

bool AssetsTreeViewModel::moveItems(const QMimeData *data, const QModelIndex &parentIndex) {
    
    //todo get topmost
    auto topMostIndexes = this->_getTopMostIndexesFromDraggedIndexesBuffer();
    auto insertInstr = this->_anticipateInserts(topMostIndexes);

    //begin removes...
    for(auto &i : topMostIndexes) {
        this->beginRemoveRows(i.parent(), i.row(), i.row());
    }

    //begin inserts...
    auto parentElem = AssetsDatabaseElement::fromIndex(parentIndex);
    if(insertInstr.first) {
        this->beginInsertRows(parentIndex, 0, insertInstr.first - 1);
    }
    if(insertInstr.second) {
        auto startIndex = parentElem->childCount();
        this->beginInsertRows(parentIndex, startIndex, startIndex + insertInstr.second - 1);
    }
    
        auto droppedList = pointerListFromMimeData(data);
        auto dest = AssetsDatabaseElement::fromIndex(parentIndex);

        auto result = this->_db->moveItems(droppedList, dest);

    this->endRemoveRows();
    this->endInsertRows();

    return result;
}

bool AssetsTreeViewModel::insertAssets(QList<QUrl> &urls, const QModelIndex &parentIndex) {
    
    //data
    auto dest = AssetsDatabaseElement::fromIndex(parentIndex);
    this->beginInsertRows(parentIndex, 0, urls.count());

    //for each url, insert
    auto allResultsOK = 0;
    for(auto &url : urls) {
        auto result = this->_db->insertAsset(url, dest);
        if(result) allResultsOK++;
    }

    //end inserting
    this->endInsertRows();
    return allResultsOK == urls.count();
}

bool AssetsTreeViewModel::removeItems(const QList<QModelIndex> &itemsIndexesToRemove) {

        //create list
        QList<AssetsDatabaseElement*> items;
        for(auto &index : itemsIndexesToRemove) {
            
            this->beginRemoveRows(index.parent(), index.row(), index.row());

            items.append(
                AssetsDatabaseElement::fromIndex(index)
            );
        }
        
        auto result = this->_db->removeItems(items);
    
    this->endRemoveRows();

    return result;
}

///////////////////
/// END HELPERS ///
///////////////////


////////////////////////
/// REIMPLEMENTATION ///
////////////////////////

//index
QModelIndex AssetsTreeViewModel::index(int row, int column, const QModelIndex &parent) const {
    
    //if root element requested..
    if(!parent.isValid()) {
        return this->createIndex(row, column, (AssetsDatabaseElement*)this->_db);
    }
    
    //prepare data
    auto data = AssetsDatabaseElement::fromIndex(parent);
    return this->createIndex(row, column, data->child(row)); 
    
}

//parent
QModelIndex AssetsTreeViewModel::parent(const QModelIndex &index) const {
    
    if(!index.isValid()) return QModelIndex();

    //prepare data
    auto data = AssetsDatabaseElement::fromIndex(index);

    //if root element requested..
    if(!data || data->isRoot()) {
        return QModelIndex();
    }

    auto parent = data->parent();
    return this->createIndex(parent->row(), 0, parent);
}

//flags
Qt::ItemFlags AssetsTreeViewModel::flags(const QModelIndex &index) const {
    
    //if unvalid
    if(!index.isValid()) return 0;
    
    //if not first column
    if(index.column()) return 0;

    //if no data pointer, return
    auto data = AssetsDatabaseElement::fromIndex(index);
    if(!data) return 0;

    //fetch flags
    return data->flags();
}

//data
QVariant AssetsTreeViewModel::data(const QModelIndex &index, int role) const {
    
    //bound data...
    auto data = AssetsDatabaseElement::fromIndex(index);

    //if no data pointer, return default
    if(!data) return QVariant();

    //for handled roles
    switch (role) {
        case Qt::DisplayRole: {
            switch(index.column()) {
                case 0:
                    return data->displayName();
                case 1:
                    return data->isContainer() ? QString::number(data->itemChildrenCount()) : QVariant();
            }
        }
        break;

        case Qt::DecorationRole: {
            
            //if not first index
            auto colIndex = index.column();
            if(colIndex) return QVariant();

            //if has iconPath
            auto iconPath = data->iconPath();
            if(!iconPath.isEmpty()) return QIcon(iconPath);

            auto type = data->type();
            QSize defaultQSize = type == AssetsDatabaseElement::Type::FloorBrush ? QSize(32, 32) : QSize(55, 55);

            switch(type) {
                case AssetsDatabaseElement::Type::NPC:
                case AssetsDatabaseElement::Type::FloorBrush:
                case AssetsDatabaseElement::Type::Object:
                case AssetsDatabaseElement::Type::Downloaded: {  
                    
                    auto cachedPixmap = getAssetIcon(data, defaultQSize);
                    if(!cachedPixmap.isNull()) return cachedPixmap;

                    return QVariant();

                }
                break;
                
                default:
                    return QVariant();
            }

        }
        break;

        case Qt::SizeHintRole: {
            auto type = data->type();
            if(type == AssetsDatabaseElement::Type::NPC || type == AssetsDatabaseElement::Type::Object || type == AssetsDatabaseElement::Type::Downloaded)  {
                return QSize(0, 64);
            }
            return QVariant();
        }
        break;

        default:
            return QVariant();
    }
    
    return QVariant();
}

bool AssetsTreeViewModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    
    //bound data...
    auto data = AssetsDatabaseElement::fromIndex(index);

    //if not main column, default behavior
    if(index.column()) return QAbstractItemModel::setData(index, value, role);

    switch(role) {
        case Qt::EditRole: {
            auto name = value.toString();
            return this->_db->rename(name, data);
        }
        break;

        default:
            return QAbstractItemModel::setData(index, value, role);
    }
}

int AssetsTreeViewModel::columnCount(const QModelIndex &parent) const {
    return 2;
}

int AssetsTreeViewModel::rowCount(const QModelIndex &parent) const {

    //is root
    if(!parent.isValid()) {
        return 1;
    }

    //is inner element
    auto data = AssetsDatabaseElement::fromIndex(parent);
    return data->childCount();
}

////////////////////////////
/// END REIMPLEMENTATION ///
////////////////////////////

/////////////////////
/// DROP HANDLING ///
/////////////////////

bool AssetsTreeViewModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) {
    
    if(data->hasFormat(AssetsDatabaseElement::listMimeType)) {

        //inner drop, move
        return this->moveItems(data, parent);
    
    } else if(data->hasFormat("text/uri-list")){
    
        //external drop, insert 
        auto includedUrls = data->urls();
        return this->insertAssets(includedUrls, parent);
    }

    //should not happen
    return false;
}

QList<AssetsDatabaseElement*> AssetsTreeViewModel::pointerListFromMimeData(const QMimeData *data) {

    //get bound data
    auto asString = QString::fromUtf8(
        data->data(AssetsDatabaseElement::listMimeType)
    );
    auto pointerList = asString.split(";");

    //iterate through list
    QList<AssetsDatabaseElement*> list;
    for(auto &pointerAsString : pointerList) {
        auto ptr = (AssetsDatabaseElement*)pointerAsString.toLongLong();
        list << ptr;
    }

    return list;
}

bool AssetsTreeViewModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const {

    //prevent dropping anywhere else than in the main column
    if(column > 0) {
        return false;
    }

    //external drop, allows
    if(!data->hasFormat(AssetsDatabaseElement::listMimeType)) {
        return true;
    }

    //if is root or not corresponding to a tree element
    auto dest = AssetsDatabaseElement::fromIndex(parent);
    if(!dest || dest->isRoot()) {
        return false;
    }

    //iterate through list
    auto dataList = pointerListFromMimeData(data);
    for(auto &ptr : dataList) {

        //internal drop/move, reject internal objects drop
        if(ptr->isInternal()) {
            return false;
        }

    }

    //prevent dropping into a selected index
    QList<QString> selectedPaths;
    for(auto &ptr : dataList) {
        selectedPaths.append(ptr->path());
    }
    auto destPath = dest->path();
    if(selectedPaths.contains(destPath)) {
        return false;
    }


    //else, allow
    return true;
}

QMimeData* AssetsTreeViewModel::mimeData(const QModelIndexList &indexes) const {
    
    //create pointer list
    QString pointerList;
    for(auto &i : indexes) {

        //only first column
        if(i.column() > 0) continue;

        //add to list
        pointerList += QString::number((long long)i.internalPointer()) + ";";
    }
    //remove last separator
    pointerList = pointerList.left(pointerList.length() - 1); 

    //update mime
    auto mimeData = QAbstractItemModel::mimeData(indexes);
    mimeData->setData(AssetsDatabaseElement::listMimeType, pointerList.toUtf8());

    //store as dragged indexes
    this->_bufferedDraggedIndexes = indexes;

    return mimeData;
}

Qt::DropActions AssetsTreeViewModel::supportedDropActions() const {
    return (
        Qt::DropAction::CopyAction | Qt::DropAction::MoveAction
    );
}

/////////////////////////
/// END DROP HANDLING ///
/////////////////////////