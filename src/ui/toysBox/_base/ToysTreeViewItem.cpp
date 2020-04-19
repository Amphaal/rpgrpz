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

#include "ToysTreeViewItem.h"

ToysTreeViewItem::ToysTreeViewItem(ToysTreeViewItem* ancestor, const ToysTreeViewItem::Type &type, const QString &name) {
    // define type
    this->_setType(type);

    // define name (fullpath redefinition included)
    this->rename(name);

    // if a ancestor is defined, add self to its inner list
    if (ancestor) {
        ancestor->appendSubItem(this);
    }
}

ToysTreeViewItem::ToysTreeViewItem(ToysTreeViewItem* ancestor, const QString &folderName) :
    ToysTreeViewItem(ancestor, ToysTreeViewItem::Type::Folder, folderName) {}

ToysTreeViewItem::ToysTreeViewItem(ToysTreeViewItem* ancestor, const ToysTreeViewItem::Type &type) :
    ToysTreeViewItem(ancestor, type, ToysTreeViewItem::typeDescription(type)) {}

ToysTreeViewItem::ToysTreeViewItem(ToysTreeViewItem* ancestor, const RPZAsset* asset) : ToysTreeViewItem(ancestor, ancestor->insertType(), asset->name()) {
    this->_hash = asset->hash();
}

ToysTreeViewItem::ToysTreeViewItem() : ToysTreeViewItem(nullptr, ToysTreeViewItem::Type::Root) {}

ToysTreeViewItem::~ToysTreeViewItem() {
    if (this->_ancestor) {
        this->_ancestor->unrefSubItem(this);
    }

    qDeleteAll(this->_subItems);
}

void ToysTreeViewItem::_setType(const ToysTreeViewItem::Type &type) {
    this->_type = type;

    // types-related definitions
    this->_defineIconPath();
    this->_defineFlags();
    this->_defineIsContainer();
    this->_defineIsRoot();
    this->_defineIsStaticContainer();
    this->_defineIsDeletable();
    this->_defineIsInvokable();
    this->_defineAllowingSubFolderCreation();
}

///////////////////
// RO Properties //
///////////////////

const RPZAsset* ToysTreeViewItem::asset() const {
    return AssetsDatabase::get()->asset(this->_hash);
}
const RPZAsset ToysTreeViewItem::assetCopy() const {
    auto asset = this->asset();
    if (asset) return *asset;
    return RPZAsset();
}

const Qt::ItemFlags ToysTreeViewItem::flags() const {
    return this->_flags;
}

bool ToysTreeViewItem::isInvokable() const {
    return this->_isInvokable;
}

const QString ToysTreeViewItem::displayName() const {
    return this->_name;
}

ToysTreeViewItem::Type ToysTreeViewItem::type() const {
    return this->_type;
}

ToysTreeViewItem* ToysTreeViewItem::ancestor() {
    return this->_ancestor;
}

const QString ToysTreeViewItem::path() const {
    return this->_path;
}

const QString ToysTreeViewItem::fullPath() const {
    return this->_fullPath;
}

const QString ToysTreeViewItem::iconPath() const {
    return this->_iconPath;
}

bool ToysTreeViewItem::isContainer() const {
    return this->_isContainer;
}

bool ToysTreeViewItem::isRoot() const {
    return this->_isRoot;
}

bool ToysTreeViewItem::isDeletable() const {
    return this->_isDeletable;
}

ToysTreeViewItem::Type ToysTreeViewItem::insertType() const {
    return this->_insertType;
}

ToysTreeViewItem::Type ToysTreeViewItem::rootStaticContainer() const {
    return this->_rootStaticContainerType;
}

bool ToysTreeViewItem::allowsSubFolderCreation() const {
    return this->_allowsSubFolderCreation;
}

///////////////////////
// END RO Properties //
///////////////////////

bool ToysTreeViewItem::contains(ToysTreeViewItem* toCheck, ToysTreeViewItem* toBeChecked) {
    if (!toBeChecked) toBeChecked = this;

    // check if self
    if (toBeChecked == toCheck) return true;

    // check subItems
    auto currentDoesContainInSubItemren = toBeChecked->_subItems.contains(toCheck);
    if (currentDoesContainInSubItemren) return true;

    return false;
}

bool ToysTreeViewItem::containsAny(const QList<ToysTreeViewItem*> toCheck) {
    for (const auto i : toCheck) {
        auto doesContain = this->contains(i);
        if (doesContain) return true;
    }

    return false;
}

ToysTreeViewItem* ToysTreeViewItem::subItem(int row) {
    return this->_subItems.value(row);
}

int ToysTreeViewItem::subItemCount() const {
    return this->_subItems.count();
}

int ToysTreeViewItem::toySubItemCount() const {
    return this->_toySubItemCount;
}

int ToysTreeViewItem::row() const {
    if (this->_ancestor) {
        return this->_ancestor->_subItems.indexOf(const_cast<ToysTreeViewItem*>(this));
    }

    return 0;
}

void ToysTreeViewItem::appendSubItem(ToysTreeViewItem* subItem) {
    subItem->_defineAncestor(this);

    // add to list
    if (subItem->type() == ToysTreeViewItem::Type::Folder) {
        this->_subItems.prepend(subItem);
    } else {
        this->_subItems.append(subItem);
    }

    // increment count
    if (!subItem->isContainer()) this->_toySubItemCount++;
}

void ToysTreeViewItem::unrefSubItem(ToysTreeViewItem* subItem) {
    // find subItem in subelements
    auto foundIndex = this->_subItems.indexOf(subItem);

    // if found
    if (foundIndex > -1) {
        // unref
        this->_subItems.removeAt(foundIndex);

        // unincrement
        if (!subItem->isContainer()) _toySubItemCount--;
    }
}

QList<ToysTreeViewItem*> ToysTreeViewItem::containerSubItems() {
    QList<ToysTreeViewItem*> list;

    for (const auto &elem : this->_subItems) {
        if (elem->isContainer()) list.append(elem);
    }

    return list;
}

QList<ToysTreeViewItem*> ToysTreeViewItem::toySubItems() {
    auto filterType = this->insertType();

    QList<ToysTreeViewItem*> list;
    for (const auto &elem : this->_subItems) {
        if (filterType == elem->type()) list.append(elem);
    }

    return list;
}

void ToysTreeViewItem::rename(const QString &newName) {
    this->_name = newName;

    // redefine paths
    this->_definePath();
}


//////////////
/// DEFINES //
//////////////

void ToysTreeViewItem::_defineFlags() {
    // flags definition
    switch (this->_type) {
        case ToysTreeViewItem::Type::InternalContainer:
        case ToysTreeViewItem::Type::DownloadedContainer:
            this->_flags = QFlags<Qt::ItemFlag>(Qt::ItemIsEnabled);
            break;

        // disabled for now
        case ToysTreeViewItem::Type::Text:
        case ToysTreeViewItem::Type::BackgroundContainer:
        case ToysTreeViewItem::Type::Background:
            this->_flags = QFlags<Qt::ItemFlag>(Qt::ItemNeverHasChildren);
            break;

        case ToysTreeViewItem::Type::POI:
        case ToysTreeViewItem::Type::Event:
        case ToysTreeViewItem::Type::FreeDraw:
        case ToysTreeViewItem::Type::Player:
        case ToysTreeViewItem::Type::FogOfWar:
            this->_flags = QFlags<Qt::ItemFlag>(Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable);
            break;

        case ToysTreeViewItem::Type::Object:
        case ToysTreeViewItem::Type::FloorBrush:
        case ToysTreeViewItem::Type::NPC:
        case ToysTreeViewItem::Type::Downloaded:
        // case Background:
            this->_flags = QFlags<Qt::ItemFlag>(Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
            break;

        case ToysTreeViewItem::Type::NPC_Container:
        case ToysTreeViewItem::Type::FloorBrushContainer:
        case ToysTreeViewItem::Type::ObjectContainer:
        // case BackgroundContainer:
            this->_flags = QFlags<Qt::ItemFlag>(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled);
            break;

        case ToysTreeViewItem::Type::Folder:
            this->_flags = QFlags<Qt::ItemFlag>(Qt::ItemIsEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
            break;

        default:
            this->_flags = 0;
            break;
    }
}

void ToysTreeViewItem::_definePath() {
    // assimilated root, let default...
    if (!this->_ancestor) {
        this->_path.clear();
        return;
    }

    // generate path
    QString path;
    if (this->_isContainer) {
        // if is static, dont use name
        if (this->_isStaticContainer) {
            path = QStringLiteral(u"/{%1}").arg((int)this->type());
        } else {  // use name for other container types
            path = "/" + this->displayName();
        }
    }

    // return
    this->_path = this->_ancestor->path() + path;
    this->_defineFullPath();

    // update subItems paths
    for (const auto &elem : this->_subItems) {
        elem->_definePath();
    }
}

bool ToysTreeViewItem::_isAssetBased() {
    return !this->_hash.isEmpty();
}

void ToysTreeViewItem::_defineFullPath() {
    this->_fullPath = this->_isAssetBased() ?
                    this->_path + "/" + this->_name :
                    this->_path;
}


void ToysTreeViewItem::_defineIconPath() {
    this->_iconPath = _iconPathByElementType.value(this->_type);
}

void ToysTreeViewItem::_resetSubjacentItemsType(const ToysTreeViewItem::Type &replacingType, ToysTreeViewItem* target) {
    // update subItems
    for (const auto elem : target->_subItems) {
        // continue recursive
        if (elem->isContainer()) {
            ToysTreeViewItem::_resetSubjacentItemsType(replacingType, elem);
        } else if (!elem->_isAssetBased()) {  // stop and set new type
            elem->_setType(replacingType);
        }
    }
}

void ToysTreeViewItem::_defineAncestor(ToysTreeViewItem* ancestor) {
    // if already existing ancestor, tell him to deref subItem
    if (this->_ancestor) {
        this->_ancestor->unrefSubItem(this);

        // reset type if base insert type is different from ancestor's
        auto replacingType = ancestor->insertType();
        if (this->insertType() != replacingType) {
            // update self
            if (this->_isAssetBased()) {
                this->_setType(replacingType);
            }

            // update subItems
            ToysTreeViewItem::_resetSubjacentItemsType(replacingType, this);
        }
    }

    // set new ancestor
    this->_ancestor = ancestor;

    // paths-related redefinitions
    this->_definePath();
    this->_defineRootStaticContainer();
    this->_defineInsertType();
}

void ToysTreeViewItem::_defineIsContainer() {
    this->_isContainer = this->_staticContainerTypes.contains(this->_type) || this->_type == ToysTreeViewItem::Type::Folder;
}
void ToysTreeViewItem::_defineIsRoot() {
    this->_isRoot = this->_type == ToysTreeViewItem::Type::Root;
}
void ToysTreeViewItem::_defineIsStaticContainer() {
    this->_isStaticContainer = this->_staticContainerTypes.contains(this->_type);
}
void ToysTreeViewItem::_defineIsDeletable() {
    this->_isDeletable = _elemTypeByContainerType.values().contains(this->_type) || this->_type == ToysTreeViewItem::Type::Folder;
}

void ToysTreeViewItem::_defineIsInvokable() {
    this->_isInvokable = (
        RPZAtom::assetBasedAtom.contains((RPZAtom::Type)this->_type) ||
        _internalItemsTypes.contains(this->_type)
    );
}

void ToysTreeViewItem::_defineAllowingSubFolderCreation() {
    this->_allowsSubFolderCreation = _movableStaticContainerTypes.contains(this->_type) || this->_type == ToysTreeViewItem::Type::Folder;
}

void ToysTreeViewItem::_defineRootStaticContainer() {
    // if no ancestor, let default
    if (!this->_ancestor) {
        this->_rootStaticContainerType = ToysTreeViewItem::Type::T_Unknown;
        return;
    }

    // if self is bound
    if (this->_isStaticContainer) {
        this->_rootStaticContainerType = this->_type;
        return;
    }

    // fetch the information from ancestor
    this->_rootStaticContainerType = this->_ancestor->_isStaticContainer ?
            this->_ancestor->type() :
            this->_ancestor->rootStaticContainer();
}


void ToysTreeViewItem::_defineInsertType() {
    this->_insertType = _elemTypeByContainerType.value(
        this->_rootStaticContainerType,
        ToysTreeViewItem::Type::T_Unknown
    );
}

//////////////////
/// END DEFINES //
//////////////////

//////////////
/// HELPERS //
//////////////


QList<ToysTreeViewItem::Type> ToysTreeViewItem::staticContainerTypes() {
    return _staticContainerTypes;
}

QList<ToysTreeViewItem::Type> ToysTreeViewItem::movableStaticContainerTypes() {
    return _movableStaticContainerTypes;
}

const QList<ToysTreeViewItem::Type> ToysTreeViewItem::internalItemTypes() {
    return _internalItemsTypes;
}

const QString ToysTreeViewItem::typeDescription(const ToysTreeViewItem::Type &type) {
    return QObject::tr(qUtf8Printable(_typeDescriptions.value(type)));
}

bool ToysTreeViewItem::isAcceptableNameChange(QString newName) {
    // strip name from slashes and double quotes
    newName.replace("\"", "");
    newName.replace("/", "");

    // if empty name
    if (newName.isEmpty()) return false;

    // if same name, no changes
    if (this->displayName() == newName) return false;

    return true;
}

void ToysTreeViewItem::sortByPathLengthDesc(QList<ToysTreeViewItem*> &listToSort) {
    // sort algorythm
    struct {
        bool operator()(ToysTreeViewItem* a, ToysTreeViewItem* b) const {
            return a->_fullPath.count("/") > b->_fullPath.count("/");
        }
    } pathLength;

    // sort
    std::sort(listToSort.begin(), listToSort.end(), pathLength);
}

QList<QString> ToysTreeViewItem::pathAsList(const QString &path) {
    return path.split("/", QString::SplitBehavior::SkipEmptyParts);
}

ToysTreeViewItem::Type ToysTreeViewItem::pathChunktoType(const QString &chunk) {
    auto expected = chunk.startsWith("{") && chunk.endsWith("}");
    if (!expected) {
        qDebug() << "Assets : ignoring path, as its structure is not expected. >> " << qUtf8Printable(chunk);
        return ToysTreeViewItem::Type::T_Unknown;
    }
    // type cast and get element type
    auto cp_chunk = chunk;
    cp_chunk.replace("{", "");
    cp_chunk.replace("}", "");
    auto castOk = false;
    auto staticCType = (ToysTreeViewItem::Type)cp_chunk.toInt(&castOk);
    if (!castOk) {
        qDebug() << "Assets : ignoring path, as static container type was impossible to deduce. >> " << qUtf8Printable(cp_chunk);
        return ToysTreeViewItem::Type::T_Unknown;
    }

    return staticCType;
}

QSet<ToysTreeViewItem*> ToysTreeViewItem::filterTopMostOnly(QList<ToysTreeViewItem*> elemsToFilter) {
    QSet<ToysTreeViewItem*> higher;
    while (elemsToFilter.count()) {
        // take first
        if (!higher.count()) {
            higher.insert(elemsToFilter.takeFirst());
            continue;
        }

        // compare
        auto toCompareTo = elemsToFilter.takeFirst();
            auto compare_path = toCompareTo->_fullPath;
            auto compare_length = compare_path.length();

        auto isForeigner = true;

        // iterate
        QSet<ToysTreeViewItem*> obsoletePointers;
        for (const auto &st : higher) {
            auto st_path = st->_fullPath;
            auto st_length = st_path.length();

            auto outputArrayContainsBuffered = st_path.startsWith(compare_path);

            // if
            if (isForeigner && (outputArrayContainsBuffered || compare_path.startsWith(st_path))) {
                isForeigner = false;
            }

            // must be deleted, compared path must be prefered
            if (outputArrayContainsBuffered && st_length > compare_length) {
                obsoletePointers.insert(st);
            }
        }

        // if no presence or better than a stored one
        if (isForeigner || obsoletePointers.count()) {
            higher.insert(toCompareTo);
        }

        // if obsolete pointers have been marked, remove them from the higherList and add the compared one to it
        if (obsoletePointers.count()) {
            higher.subtract(obsoletePointers);
        }
    }

    return higher;
}

//////////////////
/// END HELPERS //
//////////////////

ToysTreeViewItem* ToysTreeViewItem::fromIndex(const QModelIndex &index) {
    auto ip = index.internalPointer();
    return static_cast<ToysTreeViewItem*>(ip);
}

QDebug operator<<(QDebug debug, const ToysTreeViewItem &c) {
    QDebugStateSaver saver(debug);
    debug.nospace() << c.type() << qUtf8Printable(QStringLiteral(", %1").arg(c.fullPath()));
    return debug;
}
