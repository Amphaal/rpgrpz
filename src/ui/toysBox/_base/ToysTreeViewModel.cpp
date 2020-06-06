// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

#include "ToysTreeViewModel.h"

ToysTreeViewModel::ToysTreeViewModel(QObject *parent) : QAbstractItemModel(parent), AlterationInteractor(Payload::Interactor::Local_AtomDB),
    _rootItem(new ToysTreeViewItem) {
    this->_injectStaticStructure();
    this->_injectDbStructure();
}

QModelIndex ToysTreeViewModel::getStaticContainerTypesIndex(const ToysTreeViewItem::Type &staticContainerType) const {
    auto indexOf = ToysTreeViewItem::staticContainerTypes().indexOf(staticContainerType);

    auto root = this->index(0, 0);
    auto out = this->index(indexOf, 0, root);

    return out;
}

///////////////
/// HELPERS ///
///////////////

const QModelIndex ToysTreeViewModel::createFolder(const QModelIndex &parentIndex) {
    auto item = ToysTreeViewItem::fromIndex(parentIndex);

    this->beginInsertRows(parentIndex, 0, 0);
        auto folderName = AssetsDatabase::get()->createFolder(item->path());
        new ToysTreeViewItem(item, folderName);
    this->endInsertRows();

    return this->index(0, 0, parentIndex);
}

bool ToysTreeViewModel::renameItem(const QString &newName, const QModelIndex &index) {
    auto item = ToysTreeViewItem::fromIndex(index);

    // check name availability
    auto isNameChangeAcceptable = item->isAcceptableNameChange(newName);
    if (!isNameChangeAcceptable) return false;

    // if asset
    if (auto asset = item->asset()) {
        AssetsDatabase::get()->renameAsset(newName, asset->hash());
        item->rename(newName);
        return true;
    } else if (item->type() == ToysTreeViewItem::Type::Folder) {  // if folder
        auto success = AssetsDatabase::get()->renameFolder(newName, item->path());
        if (success) item->rename(newName);
        return success;
    }

    return false;
}

bool ToysTreeViewModel::integrateAsset(RPZAssetImportPackage &package) {
    // get where exactly the new asset is supposed to be
    auto dlFolderIndex = this->getStaticContainerTypesIndex(ToysTreeViewItem::Type::DownloadedContainer);
    auto dlFolder = ToysTreeViewItem::fromIndex(dlFolderIndex);
    auto posToInsert = dlFolder->subItemCount();

    this->beginInsertRows(dlFolderIndex, posToInsert, posToInsert);
        auto success = AssetsDatabase::get()->importAsset(package);  // db add
        if (success) new ToysTreeViewItem(dlFolder, &package);  // model add
    this->endInsertRows();

    return success;
}

const QList<RPZAsset> ToysTreeViewModel::insertAssets(const QList<QUrl> &urls, const QModelIndex &parentIndex, bool* ok) {
    // data
    auto dest = ToysTreeViewItem::fromIndex(parentIndex);
    this->beginInsertRows(parentIndex, 0, urls.count());

    // for each url, insert
    QList<RPZAsset> okAssets;
    for (const auto &url : urls) {
        RPZAsset asset(url);
        if (!asset.isValidAsset()) continue;

        AssetsDatabase::get()->addAsset(asset, dest->path());  // db add
        new ToysTreeViewItem(dest, &asset);  // model add

        okAssets += asset;
    }

    // end inserting
    this->endInsertRows();

    // define is process has succeeded
    *ok = okAssets.count() == urls.count();

    return okAssets;
}

void ToysTreeViewModel::removeItems(const QList<QModelIndex> &itemsIndexesToRemove) {
    auto topmost = this->_getTopMostIndexes(itemsIndexesToRemove);

        // create list
        QList<AssetsDatabase::FolderPath> topmostPathsToRemove;
        QList<RPZAsset::Hash> topmostHashesToRemove;
        QList<ToysTreeViewItem*> topmostItemsToRemove;

        for (const auto &index : topmost) {
            this->beginRemoveRows(index.parent(), index.row(), index.row());

            auto item = ToysTreeViewItem::fromIndex(index);
            topmostItemsToRemove += item;

            if (item->type() == ToysTreeViewItem::Type::Folder) {
                topmostPathsToRemove += item->path();
            } else if (auto asset = item->asset()) {
                topmostHashesToRemove += asset->hash();
            }
        }

        // db remove
        AssetsDatabase::get()->removeFolders(topmostPathsToRemove);
        AssetsDatabase::get()->removeAssets(topmostHashesToRemove);

        // model remove
        for (const auto item : topmostItemsToRemove) {
            delete item;
        }

    this->endRemoveRows();
}

bool ToysTreeViewModel::moveItemsToContainer(const QModelIndex &parentIndex, const QList<QModelIndex> &indexesToMove) {
    // get topmost only
    auto topMostIndexes = this->_getTopMostIndexes(indexesToMove);

    // parent index to elem
    auto parentElem = ToysTreeViewItem::fromIndex(parentIndex);
    auto parentElemPath = parentElem->path();

    // triage
    QList<AssetsDatabase::FolderPath> folderPathsToMove;
    QList<RPZAsset::Hash> assetHashesToMove;
    QList<ToysTreeViewItem*> topmostItems;

    for (const auto &index : topMostIndexes) {
        auto item = ToysTreeViewItem::fromIndex(index);
        topmostItems += item;

        if (item->type() == ToysTreeViewItem::Type::Folder) {
            folderPathsToMove += item->path();
        } else if (auto asset = item->asset()) {
            assetHashesToMove += asset->hash();
        }

        // //indicate deletion
        this->beginRemoveRows(
            index.parent(),
            index.row(),
            index.row()
        );
    }

    // indicate expected inserts
    auto insertInstr = this->_anticipateInserts(topMostIndexes);
    if (insertInstr.first) {
        this->beginInsertRows(parentIndex, 0, insertInstr.first - 1);
    }
    if (insertInstr.second) {
        auto startIndex = parentElem->subItemCount();
        this->beginInsertRows(parentIndex, startIndex, startIndex + insertInstr.second - 1);
    }

    // move in DB
    AssetsDatabase::get()->moveAssetsTo(parentElemPath, assetHashesToMove);
    AssetsDatabase::get()->moveFoldersTo(parentElemPath, folderPathsToMove);

    // move model
    for (const auto i : topmostItems) {
        parentElem->appendSubItem(i);
    }

    this->endRemoveRows();
    this->endInsertRows();

    return topMostIndexes.count() == assetHashesToMove.count() + folderPathsToMove.count();
}

///////////////////
/// END HELPERS ///
///////////////////


////////////////////////
/// REIMPLEMENTATION ///
////////////////////////

// index
QModelIndex ToysTreeViewModel::index(int row, int column, const QModelIndex &parent) const {
    // if root element requested..
    if (!parent.isValid()) {
        return this->createIndex(row, column, this->_rootItem);
    }

    // prepare data
    auto data = ToysTreeViewItem::fromIndex(parent);
    return this->createIndex(row, column, data->subItem(row));
}

// parent
QModelIndex ToysTreeViewModel::parent(const QModelIndex &index) const {
    if (!index.isValid()) return QModelIndex();

    // prepare data
    auto data = ToysTreeViewItem::fromIndex(index);

    // if root element requested..
    if (!data || data->isRoot()) {
        return QModelIndex();
    }

    auto parent = data->ancestor();
    return this->createIndex(parent->row(), 0, parent);
}

// flags
Qt::ItemFlags ToysTreeViewModel::flags(const QModelIndex &index) const {
    // if unvalid
    if (!index.isValid()) return Qt::ItemFlag::NoItemFlags;

    // if not first column
    if (index.column()) return Qt::ItemFlag::NoItemFlags;

    // if no data pointer, return
    auto data = ToysTreeViewItem::fromIndex(index);
    if (!data) return Qt::ItemFlag::NoItemFlags;

    // fetch flags
    return data->flags();
}

// data
QVariant ToysTreeViewModel::data(const QModelIndex &index, int role) const {
    // bound data...
    auto data = ToysTreeViewItem::fromIndex(index);

    // if no data pointer, return default
    if (!data) return QVariant();

    // for handled roles
    switch (role) {
        case Qt::TextAlignmentRole: {
            return index.column() == 1 ? Qt::AlignRight : QVariant();
        }
        break;

        case Qt::DisplayRole: {
            switch (index.column()) {
                case 0: {
                    return data->displayName();
                }
                break;

                case 1: {
                    if (!data->isContainer() || data->type() == ToysTreeViewItem::Type::InternalContainer) return QVariant();
                    auto subItemsCount = data->toySubItemCount();
                    return subItemsCount ? QString::number(data->toySubItemCount()) + QStringLiteral(u"🖻") : QVariant();
                }
                break;
            }
        }
        break;

        case Qt::DecorationRole: {
            // if not first index
            auto colIndex = index.column();
            if (colIndex) return QVariant();

            // if has iconPath
            auto iconPath = data->iconPath();
            if (!iconPath.isEmpty()) return QIcon(iconPath);

            auto type = data->type();
            QSize defaultQSize = type == ToysTreeViewItem::Type::FloorBrush ? QSize(32, 32) : QSize(55, 55);

            auto asset = data->asset();
            if (asset) {
                auto cachedPixmap = RPZAsset::cachedIconPixmap(*asset, defaultQSize);
                if (!cachedPixmap.isNull()) return cachedPixmap;
            }

            return QVariant();
        }
        break;

        case Qt::SizeHintRole: {
            auto type = data->type();
            if (type == ToysTreeViewItem::Type::NPC || type == ToysTreeViewItem::Type::Object || type == ToysTreeViewItem::Type::Downloaded) {
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

bool ToysTreeViewModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    // if not main column, default behavior
    if (index.column()) return QAbstractItemModel::setData(index, value, role);

    switch (role) {
        case Qt::EditRole: {
            auto newName = value.toString();
            return this->renameItem(newName, index);
        }
        break;

        default:
            return QAbstractItemModel::setData(index, value, role);
    }
}

int ToysTreeViewModel::columnCount(const QModelIndex &parent) const {
    return 2;
}

int ToysTreeViewModel::rowCount(const QModelIndex &parent) const {
    // is root
    if (!parent.isValid()) {
        return 1;
    }

    // is inner element
    auto data = ToysTreeViewItem::fromIndex(parent);
    return data->subItemCount();
}

////////////////////////////
/// END REIMPLEMENTATION ///
////////////////////////////

/////////////////////
/// DROP HANDLING ///
/////////////////////

bool ToysTreeViewModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) {
    // internal drop
    if (data->hasFormat(ToysTreeViewItem::listMimeType)) {
        // move
        return this->moveItemsToContainer(parent, this->_bufferedDraggedIndexes);
    } else if (data->hasFormat("text/uri-list")) {  // external drop
        // insert
        bool success = false;
        auto result = this->insertAssets(data->urls(), parent, &success);

        // indicate change
        if (success) {
            AssetChangedPayload payload(result.first());
            AlterationHandler::get()->queueAlteration(this, payload);
        }

        return success;
    }

    // should not happen
    return false;
}

QList<ToysTreeViewItem*> ToysTreeViewModel::fromMimeData(const QMimeData *data) {
    // get bound data
    auto asString = QString::fromUtf8(
        data->data(ToysTreeViewItem::listMimeType)
    );
    auto pointerList = asString.split(";");

    // iterate through list
    QList<ToysTreeViewItem*> list;
    for (const auto &pointerAsString : pointerList) {
        auto ptr = reinterpret_cast<ToysTreeViewItem*>(pointerAsString.toULongLong());
        list.append(ptr);
    }

    return list;
}

bool ToysTreeViewModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const {
    // prevent dropping anywhere else than in the main column
    if (column > 0) {
        return false;
    }

    // external drop, allows
    if (data->hasFormat("text/uri-list")) {
        return true;
    }

    // if inner drop is not expected
    if (!data->hasFormat(ToysTreeViewItem::listMimeType)) {
        return false;
    }

    auto dest = ToysTreeViewItem::fromIndex(parent);

    // if is root or not corresponding to a tree element
    if (!dest || dest->isRoot()) {
        return false;
    }

    // prevent dropping into a selected index
    auto elems = fromMimeData(data);
    if (dest->containsAny(elems)) return false;

    // else, allow
    return true;
}

QMimeData* ToysTreeViewModel::mimeData(const QModelIndexList &indexes) const {
    // base mime
    auto mimeData = QAbstractItemModel::mimeData(indexes);

    // create pointer list
    QList<QString> pointersList;
    QModelIndexList filteredIndexList;

    // iterate
    for (const auto &i : indexes) {
        // only first column
        if (i.column() > 0) continue;

        auto elem = ToysTreeViewItem::fromIndex(i);

        // reject undeletable elements from drop
        if (!elem->isDeletable()) continue;

        // add to list
        pointersList += QString::number((unsigned long long)i.internalPointer());
        filteredIndexList += i;
    }

    // update mime
    if (pointersList.count()) {
        // apply data
        mimeData->setData(ToysTreeViewItem::listMimeType, pointersList.join(";").toUtf8());
    }

    // store as dragged indexes
    this->_bufferedDraggedIndexes = filteredIndexList;

    return mimeData;
}

Qt::DropActions ToysTreeViewModel::supportedDropActions() const {
    return (
        Qt::DropAction::CopyAction | Qt::DropAction::MoveAction
    );
}

/////////////////////////
/// END DROP HANDLING ///
/////////////////////////

void ToysTreeViewModel::_injectStaticStructure() {
    for (const auto &staticType : ToysTreeViewItem::staticContainerTypes()) {
        auto staticFolderItem = new ToysTreeViewItem(this->_rootItem, staticType);
        this->_staticElements.insert(staticType, staticFolderItem);

        if (staticType == ToysTreeViewItem::Type::InternalContainer) {
            for (const auto &type : ToysTreeViewItem::internalItemTypes()) {
                new ToysTreeViewItem(staticFolderItem, type);
            }
        }
    }
}


void ToysTreeViewModel::_injectDbStructure() {
    // get containers to fill, not inbetween folders
    auto containersToFill = this->_generateFolderTreeFromDb();

    // then fill with items
    this->_generateItemsFromDb(containersToFill);
}

QHash<AssetsDatabase::FolderPath, ToysTreeViewItem*> ToysTreeViewModel::_generateFolderTreeFromDb() {
    // sort the keys
    auto paths = AssetsDatabase::get()->paths().keys();
    paths.sort();

    // to be created items
    QHash<AssetsDatabase::FolderPath, ToysTreeViewItem*> containersToFill;

    // create folders arbo
    for (const auto &path : paths) {
        // split the path
        auto split = ToysTreeViewItem::pathAsList(path);

        // make sure first split is a type
        auto ctypeStr = split.takeFirst();
        auto staticCType = ToysTreeViewItem::pathChunktoType(ctypeStr);

        // get element from static source
        if (!this->_staticElements.contains(staticCType)) {
            qDebug() << "Assets : ignoring path, as the static container it points to does not exist >>" << qUtf8Printable(ctypeStr);
            continue;
        }
        auto staticContainerElem = this->_staticElements[staticCType];

        // create path
        auto lastContainer = this->_recursiveElementCreator(staticContainerElem, split);

        // append to list
        containersToFill.insert(path, lastContainer);
    }

    // returns
    return containersToFill;
}

ToysTreeViewItem* ToysTreeViewModel::_recursiveElementCreator(ToysTreeViewItem* parent, QList<QString> &pathChunks) {
    // if no more folders in path to create...
    if (!pathChunks.count()) return parent;

    // take first part
    auto part = pathChunks.takeFirst();

    // search if already exist
    ToysTreeViewItem* found = nullptr;
    for (const auto container : parent->containerSubItems()) {
        if (container->displayName() == part) {
            found = container;
            break;
        }
    }

    // if not found, create it
    if (!found) {
        found = new ToysTreeViewItem(parent, part);
    }

    // iterate through...
    return this->_recursiveElementCreator(found, pathChunks);
}


bool ToysTreeViewModel::_indexListContainsIndexOrParent(const QModelIndexList &base, const QModelIndex &index) {
    if (!index.isValid()) return false;
    if (base.contains(index)) return true;
    return this->_indexListContainsIndexOrParent(base, index.parent());
}

QModelIndexList ToysTreeViewModel::_getTopMostIndexes(QModelIndexList indexesList) {
    // ensure to be sorted
    std::sort(indexesList.begin(), indexesList.end());

    QModelIndexList higher;

    for (const auto &index : indexesList) {
        // add to higher if empty
        if (!higher.count()) {
            higher.append(index);
            continue;
        }

        // check if contains
        if (!this->_indexListContainsIndexOrParent(indexesList, index.parent())) {
            higher.append(index);
        }
    }

    return higher;
}

QPair<int, int> ToysTreeViewModel::_anticipateInserts(const QModelIndexList &tbi) {
    int insertAtBegin = 0;
    int insertAtEnd = 0;

    for (const auto &index : tbi) {
        auto elem = ToysTreeViewItem::fromIndex(index);
        if (elem->type() == ToysTreeViewItem::Type::Folder) insertAtBegin++;
        else
            insertAtEnd++;
    }

    return QPair<int, int>(insertAtBegin, insertAtEnd);
}

void ToysTreeViewModel::_generateItemsFromDb(const QHash<AssetsDatabase::FolderPath, ToysTreeViewItem*> &pathsToFillWithItems) {
    auto db_paths = AssetsDatabase::get()->paths();

    // create items for each end-containers
    for (auto i = pathsToFillWithItems.constBegin(); i != pathsToFillWithItems.constEnd(); ++i) {
        // define
        auto path = i.key();
        auto parent = i.value();

        // find items in db and create them
        for (const auto &id : db_paths.value(path)) {
            auto asset = AssetsDatabase::get()->asset(id);

            // if ID does not exist
            if (!asset) {
                qDebug() << "Assets : cannot insert an item as its ID is not found in the assets DB";
                continue;
            }

            // create
            new ToysTreeViewItem(parent, asset);
        }
    }
}
