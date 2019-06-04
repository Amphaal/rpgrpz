#include "AssetsTreeViewModel.h"

AssetsTreeViewModel::AssetsTreeViewModel(QObject *parent) : QAbstractItemModel(parent), _db(AssetsDatabase::get()) { };

AssetsDatabase* AssetsTreeViewModel::database() {
    return this->_db;
}

void AssetsTreeViewModel::onRPZClientConnecting(RPZClient * cc) {
    ClientBindable::onRPZClientConnecting(cc);

    //import asset
    QObject::connect(
        cc, &RPZClient::receivedAsset,
        [&, cc](const QVariantHash &package) {
            
            this->beginResetModel();
                auto asset_id = this->_db->importAsset(package); 
            this->endResetModel();

            cc->informAssetSucessfulInsertion(asset_id);

        }
    );
}

///////////////
/// HELPERS ///
///////////////

QString AssetsTreeViewModel::getFilePathToAsset(QModelIndex &targetIndex) {
    
    //if selected elem is no item, skip
    auto target = AssetsDatabaseElement::fromIndex(targetIndex);
    if(!target->isItem()) return NULL;

    return this->_db->getFilePathToAsset(target);
}

bool AssetsTreeViewModel::createFolder(QModelIndex &parentIndex) {
    this->beginInsertRows(parentIndex, 0, 0);
    
        auto data = AssetsDatabaseElement::fromIndex(parentIndex);
        auto result = this->_db->createFolder(data);

    this->endInsertRows();
    return result;
}

bool AssetsTreeViewModel::moveItems(const QMimeData *data, const QModelIndex &parentIndex) {
    this->beginResetModel();
    
        auto droppedList = pointerListFromMimeData(data);
        auto dest = AssetsDatabaseElement::fromIndex(parentIndex);

        auto result = this->_db->moveItems(droppedList, dest);

    this->endResetModel();

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
    
    this->beginResetModel();

        //create list
        QList<AssetsDatabaseElement*> items;
        for(auto &index : itemsIndexesToRemove) {
            
            items.append(
                AssetsDatabaseElement::fromIndex(index)
            );
        }
        
        auto result = this->_db->removeItems(items);
    
    this->endResetModel();

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
    if(!index.isValid()) {
        return 0;
    }
    
    //if column, only enabled
    if(index.column()) {
        return QFlags<Qt::ItemFlag>(Qt::ItemIsEnabled);
    }

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
        case Qt::DisplayRole:
            switch(index.column()) {
                case 0:
                    return data->displayName();
                case 1:
                    return data->isContainer() ? QString::number(data->itemChildrenCount()) : QVariant();
            }
        case Qt::DecorationRole:
            return index.column() == 0 ? QIcon(data->iconPath()) : QVariant();
        default:
            return QVariant();
    }
    
}

bool AssetsTreeViewModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    
    //bound data...
    auto data = AssetsDatabaseElement::fromIndex(index);

    //if not main column, default behavior
    if(index.column()) return QAbstractItemModel::setData(index, value, role);

    switch(role) {
        case Qt::EditRole:
            return this->_db->rename(value.toString(), data);
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
        return this->insertAssets(data->urls(), parent);
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
    if(column > 0) return false;

    //external drop, allows
    if(!data->hasFormat(AssetsDatabaseElement::listMimeType)) return true;

    //if is root or not corresponding to a tree element
    auto dest = AssetsDatabaseElement::fromIndex(parent);
    if(!dest || dest->isRoot()) return false;

    //iterate through list
    auto dataList = pointerListFromMimeData(data);
    for(auto &ptr : dataList) {
        //internal drop/move, reject internal objects drop
        if(ptr->isInternal()) return false;
    }

    //prevent dropping into a selected index
    QList<QString> selectedPaths;
    for(auto &ptr : dataList) {
        selectedPaths.append(ptr->path());
    }
    auto destPath = dest->path();
    if(selectedPaths.contains(destPath)) return false;

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

    //mime
    auto mimeData = QAbstractItemModel::mimeData(indexes);
    mimeData->setData(AssetsDatabaseElement::listMimeType, pointerList.toUtf8());
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