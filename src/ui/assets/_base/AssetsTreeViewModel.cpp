#include "AssetsTreeViewModel.h"

AssetsTreeViewModel::AssetsTreeViewModel(QObject *parent) : QAbstractItemModel(parent), _rootItem(new AssetsTreeViewItem) {
    this->_injectStaticStructure();
    this->_injectDbStructure();
};

QModelIndex AssetsTreeViewModel::getStaticContainerTypesIndex(const AssetsTreeViewItem::Type &staticContainerType) {
    
    auto indexOf = AssetsTreeViewItem::staticContainerTypes().indexOf(staticContainerType);
    
    auto root = this->index(0, 0);
    auto out = this->index(indexOf, 0, root);

    return out;
}

bool AssetsTreeViewModel::integrateAsset(RPZAssetImportPackage &package) {
    
    //get where exactly the new asset is supposed to be
    auto dlFolderIndex = this->getStaticContainerTypesIndex(AssetsTreeViewItem::Type::DownloadedContainer);
    auto dlFolder = AssetsTreeViewItem::fromIndex(dlFolderIndex);
    auto posToInsert = dlFolder->childCount();
    
    this->beginInsertRows(dlFolderIndex, posToInsert, posToInsert);
        auto success = AssetsDatabase::get()->importAsset(package); 
    this->endInsertRows();

    return success;
}

///////////////
/// HELPERS ///
///////////////

void AssetsTreeViewModel::createFolder(QModelIndex &parentIndex) {
    this->beginInsertRows(parentIndex, 0, 0);
    
        auto item = AssetsTreeViewItem::fromIndex(parentIndex);
        AssetsDatabase::get()->createFolder(item->path());

    this->endInsertRows();
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

    //triage
    QList<RPZFolderPath> folderPathsToMove;
    QList<RPZAssetHash> assetHashesToMove;
    for(auto &index : topMostIndexes) {
        
        auto item = AssetsTreeViewItem::fromIndex(index);

        if(item->type() == AssetsTreeViewItem::Type::Folder) {
            folderPathsToMove += item->path();
        } 
        
        else if(auto asset = item->asset()) {
            assetHashesToMove += asset->hash();
        }

    }

    //parent index to elem
    auto parentElem = AssetsTreeViewItem::fromIndex(parentIndex);
    auto parentElemPath = parentElem->path();

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
    AssetsDatabase::get()->moveAssetsTo(parentElemPath, assetHashesToMove);
    AssetsDatabase::get()->moveFoldersTo(parentElemPath, folderPathsToMove);

    this->endRemoveRows();
    this->endInsertRows();

    return topMostIndexes.count() == assetHashesToMove.count() + folderPathsToMove.count();

}


bool AssetsTreeViewModel::insertAssets(QList<QUrl> &urls, const QModelIndex &parentIndex) {
    
    //data
    auto dest = AssetsTreeViewItem::fromIndex(parentIndex);
    this->beginInsertRows(parentIndex, 0, urls.count());

    //for each url, insert
    auto allResultsOK = 0;
    for(auto &url : urls) {

        RPZAsset asset(url);
        if(!asset.isValidAsset()) continue;
        
        AssetsDatabase::get()->addAsset(asset, dest->path());
        allResultsOK++;

    }

    //end inserting
    this->endInsertRows();
    return allResultsOK == urls.count();

}

void AssetsTreeViewModel::removeItems(const QList<QModelIndex> &itemsIndexesToRemove) {
    
    auto topmost = this->_getTopMostIndexes(itemsIndexesToRemove);

        //create list
        QList<RPZFolderPath> pathsToRemove;
        QList<RPZAssetHash> hashesToRemove;

        for(auto &index : topmost) {
            this->beginRemoveRows(index.parent(), index.row(), index.row());
            auto item = AssetsTreeViewItem::fromIndex(index);
            
            if(item->type() == AssetsTreeViewItem::Type::Folder) {
                pathsToRemove += item->path();
            } 
            
            else if(auto asset = item->asset()) {
                hashesToRemove += asset->hash();
            }

        }

        AssetsDatabase::get()->removeFolders(pathsToRemove);
        AssetsDatabase::get()->removeAssets(hashesToRemove);
    
    this->endRemoveRows();

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
        return this->createIndex(row, column, this->_rootItem);
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

            auto asset = data->asset();
            if(asset) {
                auto cachedPixmap = RPZAsset::cachedIconPixmap(*asset, defaultQSize);
                if(cachedPixmap) return *cachedPixmap;
            }
                    
            return QVariant();

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
    auto item = AssetsTreeViewItem::fromIndex(index);

    //if not main column, default behavior
    if(index.column()) return QAbstractItemModel::setData(index, value, role);

    switch(role) {
        
        case Qt::EditRole: {

            auto newName = value.toString();

            if(auto asset = item->asset()) {
                AssetsDatabase::get()->renameAsset(newName, asset->hash());
                return true;
            }

            else if(item->type() == AssetsTreeViewItem::Type::Folder) {
                AssetsDatabase::get()->renameFolder(newName, item->path());
            }

            return false;

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

void AssetsTreeViewModel::_injectStaticStructure() {

    for(auto &staticType : AssetsTreeViewItem::staticContainerTypes()) {

        auto staticFolderItem = new AssetsTreeViewItem(this->_rootItem, staticType);
        this->_staticElements.insert(staticType, staticFolderItem);

        if(staticType == AssetsTreeViewItem::Type::InternalContainer) {
            
            for(auto &type : AssetsTreeViewItem::internalItemTypes()) {
                auto internalItem = new AssetsTreeViewItem(staticFolderItem, type);
            }

        }

    }

}


void AssetsTreeViewModel::_injectDbStructure() {

    //get containers to fill, not inbetween folders
    auto containersToFill = this->_generateFolderTreeFromDb();

    //then fill with items
    this->_generateItemsFromDb(containersToFill);

}

QHash<RPZFolderPath, AssetsTreeViewItem*> AssetsTreeViewModel::_generateFolderTreeFromDb() {
    
    //sort the keys
    auto paths = AssetsDatabase::get()->paths().keys();
    paths.sort();

    //to be created items
    QHash<RPZFolderPath, AssetsTreeViewItem*> containersToFill;

    //create folders arbo
    for(auto &path : paths) {
        
        //split the path
        auto split = AssetsTreeViewItem::pathAsList(path);
        
        //make sure first split is a type
        auto staticCType = AssetsTreeViewItem::pathChunktoType(split.takeFirst());

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

AssetsTreeViewItem* AssetsTreeViewModel::_recursiveElementCreator(AssetsTreeViewItem* parent, QList<QString> &pathChunks) {
    
    //if no more folders in path to create...
    if(!pathChunks.count()) return parent;

    //take first part
    auto part = pathChunks.takeFirst();

    //search if already exist
    AssetsTreeViewItem* found = nullptr;
    for(auto container : parent->childrenContainers()) {
        if(container->displayName() == part) {
            found = container;
            break;
        }
    }

    //if not found, create it
    if(!found) {
        found = new AssetsTreeViewItem(parent, part);
    }

    //iterate through...
    return this->_recursiveElementCreator(found, pathChunks);

}

void AssetsTreeViewModel::_generateItemsFromDb(const QHash<RPZFolderPath, AssetsTreeViewItem*> &pathsToFillWithItems) {
    
    auto db_paths = AssetsDatabase::get()->paths();

    //create items for each end-containers
    for (auto i = pathsToFillWithItems.constBegin(); i != pathsToFillWithItems.constEnd(); ++i) {
        
        //define
        auto path = i.key();
        auto parent = i.value();
        
        //find items in db and create them
        for(auto &id : db_paths.value(path)) {

            auto asset = AssetsDatabase::get()->asset(id);

            //if ID doesnt exist
            if(!asset) {
                qDebug() << "Assets : cannot insert an item as its ID is not found in the assets DB";
                continue;
            }

            //create
            auto elem = new AssetsTreeViewItem(parent, asset);

        }
    }

}