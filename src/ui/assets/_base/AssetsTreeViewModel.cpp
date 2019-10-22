#include "AssetsTreeViewModel.h"

AssetsTreeViewModel::AssetsTreeViewModel(QObject *parent) : 
    QAbstractItemModel(parent), 
    _db(AssetsDatabase::get()) { };

AssetsDatabase* AssetsTreeViewModel::database() const {
    return this->_db;
}

QModelIndex AssetsTreeViewModel::getStaticContainerTypesIndex(const AssetsTreeViewItem::Type &staticContainerType) {
    
    auto indexOf = AssetsTreeViewItem::staticContainerTypes().indexOf(staticContainerType);
    
    auto root = this->index(0, 0);
    auto out = this->index(indexOf, 0, root);

    return out;
}

RPZToyMetadata AssetsTreeViewModel::integrateAsset(const RPZAssetImportPackage &package) {
    
    //get where exactly the new asset is supposed to be
    auto dlFolderIndex = this->getStaticContainerTypesIndex(AssetsTreeViewItem::Type::DownloadedContainer);
    auto dlFolder = AssetsTreeViewItem::fromIndex(dlFolderIndex);
    auto posToInsert = dlFolder->childCount();
    
    this->beginInsertRows(dlFolderIndex, posToInsert, posToInsert);
        auto metadata = this->_db->importAsset(package); 
    this->endInsertRows();

    return metadata;
}

///////////////
/// HELPERS ///
///////////////

QPixmap AssetsTreeViewModel::getAssetIcon(AssetsTreeViewItem* target, QSize &sizeToApply) const {
    
    QPixmap toFind;

    //if selected elem is no item, skip
    if(!target->isIdentifiable()) return toFind;

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
    
        auto data = AssetsTreeViewItem::fromIndex(parentIndex);
        auto result = this->_db->createFolder(data);

    this->endInsertRows();
    return result;
}

bool AssetsTreeViewModel::_indexListContainsIndexOrParent(const QModelIndexList &base, const QModelIndex &index) {
    if(!index.isValid()) return false;
    if(base.contains(index)) return true;
    return this->_indexListContainsIndexOrParent(base, index.parent());
}

QModelIndexList AssetsTreeViewModel::_getTopMostIndexes(const QModelIndexList &indexesList) {
    
    QModelIndexList higher;

    for(auto &index : indexesList) {

        //add to higher if empty
        if(!higher.count()) {
            higher.append(index);
            continue;
        }

        //check if contains 
        if(!this->_indexListContainsIndexOrParent(indexesList, index.parent())) {
            higher.append(index);
        }
        
    }

    return higher;
}

QPair<int, int> AssetsTreeViewModel::_anticipateInserts(const QModelIndexList &tbi) {
    
    int insertAtBegin = 0;
    int insertAtEnd = 0;
    
    for(auto &index : tbi) {
        auto elem = AssetsTreeViewItem::fromIndex(index);
        if(elem->type() == AssetsTreeViewItem::Type::Folder) insertAtBegin++;
        else insertAtEnd++;
    }

    return QPair<int, int>(insertAtBegin, insertAtEnd);
}

bool AssetsTreeViewModel::moveItemsToContainer(const QModelIndex &parentIndex, const QList<QModelIndex> &indexesToMove) {
    
    //get topmost only
    auto topMostIndexes = this->_getTopMostIndexes(indexesToMove);

    //to move indexes to elem list
    QList<AssetsTreeViewItem*> elementsToMove;
    for(auto &index : topMostIndexes) elementsToMove += AssetsTreeViewItem::fromIndex(index);

    //parent index to elem
    auto parentElem = AssetsTreeViewItem::fromIndex(parentIndex);

    //begin removes...
    for(auto &i : topMostIndexes) {
        this->beginRemoveRows(
            i.parent(), 
            i.row(), 
            i.row()
        );
    }
    
    //insert row instructions
    auto insertInstr = this->_anticipateInserts(topMostIndexes);
    if(insertInstr.first) {
        this->beginInsertRows(parentIndex, 0, insertInstr.first - 1);
    }
    if(insertInstr.second) {
        auto startIndex = parentElem->childCount();
        this->beginInsertRows(parentIndex, startIndex, startIndex + insertInstr.second - 1);
    }

    //move
    auto result = this->_db->moveItemsToContainer(elementsToMove, parentElem);

    this->endRemoveRows();
    this->endInsertRows();

    return result;

}


bool AssetsTreeViewModel::insertAssets(QList<QUrl> &urls, const QModelIndex &parentIndex) {
    
    //data
    auto dest = AssetsTreeViewItem::fromIndex(parentIndex);
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
        QList<AssetsTreeViewItem*> items;
        for(auto &index : itemsIndexesToRemove) {
            
            this->beginRemoveRows(index.parent(), index.row(), index.row());

            items.append(
                AssetsTreeViewItem::fromIndex(index)
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
        return this->createIndex(row, column, (AssetsTreeViewItem*)this->_db);
    }
    
    //prepare data
    auto data = AssetsTreeViewItem::fromIndex(parent);
    return this->createIndex(row, column, data->child(row)); 
    
}

//parent
QModelIndex AssetsTreeViewModel::parent(const QModelIndex &index) const {
    
    if(!index.isValid()) return QModelIndex();

    //prepare data
    auto data = AssetsTreeViewItem::fromIndex(index);

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
    auto data = AssetsTreeViewItem::fromIndex(index);
    if(!data) return 0;

    //fetch flags
    return data->flags();
}

//data
QVariant AssetsTreeViewModel::data(const QModelIndex &index, int role) const {
    
    //bound data...
    auto data = AssetsTreeViewItem::fromIndex(index);

    //if no data pointer, return default
    if(!data) return QVariant();

    //for handled roles
    switch (role) {
        
        case Qt::TextAlignmentRole: {
            return index.column() == 1 ? Qt::AlignRight : QVariant();
        }
        break;
        
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
            QSize defaultQSize = type == AssetsTreeViewItem::Type::FloorBrush ? QSize(32, 32) : QSize(55, 55);

            switch(type) {
                case AssetsTreeViewItem::Type::NPC:
                case AssetsTreeViewItem::Type::FloorBrush:
                case AssetsTreeViewItem::Type::Object:
                case AssetsTreeViewItem::Type::Background:
                case AssetsTreeViewItem::Type::Downloaded: {  
                    
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
            if(type == AssetsTreeViewItem::Type::NPC || type == AssetsTreeViewItem::Type::Object || type == AssetsTreeViewItem::Type::Downloaded)  {
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
    auto data = AssetsTreeViewItem::fromIndex(index);

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
    auto data = AssetsTreeViewItem::fromIndex(parent);
    return data->childCount();
}

////////////////////////////
/// END REIMPLEMENTATION ///
////////////////////////////

/////////////////////
/// DROP HANDLING ///
/////////////////////

bool AssetsTreeViewModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) {
    
    //internal drop
    if(data->hasFormat(AssetsTreeViewItem::listMimeType)) {

        //move
        return this->moveItemsToContainer(parent, this->_bufferedDraggedIndexes);
    
    } 
    
    //external drop
    else if(data->hasFormat("text/uri-list")){
    
        //insert 
        auto includedUrls = data->urls();
        return this->insertAssets(includedUrls, parent);

    }

    //should not happen
    return false;

}

QList<AssetsTreeViewItem*> AssetsTreeViewModel::fromMimeData(const QMimeData *data) {

    //get bound data
    auto asString = QString::fromUtf8(
        data->data(AssetsTreeViewItem::listMimeType)
    );
    auto pointerList = asString.split(";");

    //iterate through list
    QList<AssetsTreeViewItem*> list;
    for(auto &pointerAsString : pointerList) {
        auto ptr = (AssetsTreeViewItem*)pointerAsString.toULongLong();
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
    if(data->hasFormat("text/uri-list")) {
        return true;
    }

    //if inner drop is not expected
    if(!data->hasFormat(AssetsTreeViewItem::listMimeType)) {
        return false;
    }

    
    auto dest = AssetsTreeViewItem::fromIndex(parent);

    //if is root or not corresponding to a tree element
    if(!dest || dest->isRoot()) {
        return false;
    }

    //prevent dropping into a selected index
    auto elems = fromMimeData(data);
    if(dest->containsAny(elems)) return false;

    //else, allow
    return true;
}

QMimeData* AssetsTreeViewModel::mimeData(const QModelIndexList &indexes) const {
    
    //base mime
    auto mimeData = QAbstractItemModel::mimeData(indexes);

    //create pointer list
    QList<QString> pointersList;
    QModelIndexList filteredIndexList;

    //iterate
    for(auto &i : indexes) {

        //only first column
        if(i.column() > 0) continue;
        
        auto elem = AssetsTreeViewItem::fromIndex(i);
        
        //reject undeletable elements from drop
        if(!elem->isDeletable()) continue;

        //add to list
        pointersList += QString::number((unsigned long long)i.internalPointer());
        filteredIndexList += i;

    }

    //update mime
    if(pointersList.count()) {

        //apply data
        mimeData->setData(AssetsTreeViewItem::listMimeType, pointersList.join(";").toUtf8());

    }

    //store as dragged indexes
    this->_bufferedDraggedIndexes = filteredIndexList;

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