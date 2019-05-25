#include "AssetsDatabaseElement.h"

AssetsDatabaseElement* AssetsDatabaseElement::fromIndex(QModelIndex index) {
    return static_cast<AssetsDatabaseElement*>(index.internalPointer());
}

AssetsDatabaseElement::AssetsDatabaseElement() : AssetsDatabaseElement(QString(""), nullptr, Root) { };

AssetsDatabaseElement::~AssetsDatabaseElement(){
    
    if(this->_parentElement) {
        this->_parentElement->unrefChild(this);
    }

    qDeleteAll(this->_subElements);
}

AssetsDatabaseElement::AssetsDatabaseElement(
    QString &name, 
    AssetsDatabaseElement* parent,
    const AssetsDatabaseElement::Type &type,
    QString id
) { 
    //define type
    this->_setType(type);

    //define name (fullpath redefinition included)
    this->rename(name);
 
    //prevent id definition if not asset Type
    if(this->isItem()) {
        this->_id = id;
    }

   //if a parent is defined, add self to its inner list
    if(parent) {
        parent->appendChild(this);
    }

};

void AssetsDatabaseElement::_setType(const AssetsDatabaseElement::Type &type) {
    this->_type = type; 

      // types-related definitions
    this->_defineIconPath();
    this->_defineFlags();
    this->_defineIsContainer();
    this->_defineIsRoot();
    this->_defineIsItem();
    this->_defineIsStaticContainer();
    this->_defineIsDeletable();
}

///////////////////
// RO Properties //
///////////////////

Qt::ItemFlags AssetsDatabaseElement::flags() {
    return this->_flags;
}

QString AssetsDatabaseElement::displayName() {
    return this->_name;
}

QString AssetsDatabaseElement::id() {
    return this->_id;
}

AssetsDatabaseElement::Type AssetsDatabaseElement::type() {
    return this->_type;
}

AssetsDatabaseElement* AssetsDatabaseElement::parent() {
    return this->_parentElement;
}

QString AssetsDatabaseElement::fullPath() {
    return this->_fullPath;
}

QString AssetsDatabaseElement::path() {
    return this->_path;
}

QString AssetsDatabaseElement::iconPath() {
    return this->_iconPath;
}

bool AssetsDatabaseElement::isContainer() {
    return this->_isContainer;
}

bool AssetsDatabaseElement::isInternal() {
    return this->_isInternal;
}

bool AssetsDatabaseElement::isRoot() {
    return this->_isRoot;
}

bool AssetsDatabaseElement::isItem() {
    return this->_isItem;
}

bool AssetsDatabaseElement::isStaticContainer() {
    return this->_isStaticContainer;
}

bool AssetsDatabaseElement::isDeletable() {
    return this->_isDeletable;
}

AssetsDatabaseElement::Type AssetsDatabaseElement::insertType() {
    return this->_insertType;
}

AssetsDatabaseElement::Type AssetsDatabaseElement::rootStaticContainer() {
    return this->_rootStaticContainerType;
}


///////////////////////
// END RO Properties //
///////////////////////


AssetsDatabaseElement* AssetsDatabaseElement::child(int row) {
    return this->_subElements.value(row);
}

int AssetsDatabaseElement::childCount() const {
    return this->_subElements.count();
}

int AssetsDatabaseElement::itemChildrenCount() const {
    return this->_itemChildrenCount;
}

int AssetsDatabaseElement::row() const {
    if (this->_parentElement) {
        return this->_parentElement->_subElements.indexOf(const_cast<AssetsDatabaseElement*>(this));
    }

    return 0;
}

void AssetsDatabaseElement::appendChild(AssetsDatabaseElement* child) {
    
    child->_defineParent(this);

    //add to list
    if(child->type() == Folder) {
         this->_subElements.prepend(child);
    } else {
        this->_subElements.append(child);
    }
    
    //increment count
    if(child->isItem()) this->_itemChildrenCount++;
};


void AssetsDatabaseElement::unrefChild(AssetsDatabaseElement* child) {

    //find child in subelements
    auto foundIndex = this->_subElements.indexOf(child);
    
    //if found
    if (foundIndex > -1) {

        //unref
        this->_subElements.removeAt(foundIndex);

        //unincrement
        if(child->isItem()) _itemChildrenCount--;
    }
}

QList<AssetsDatabaseElement*> AssetsDatabaseElement::childrenContainers() {
    QList<AssetsDatabaseElement*> list;
    
    for(auto &elem : this->_subElements) {
        if(elem->isContainer()) list.append(elem);
    }

    return list;
}

QList<AssetsDatabaseElement*> AssetsDatabaseElement::childrenItems() {
    QList<AssetsDatabaseElement*> list;
    auto filterType = this->insertType();
    
    for(auto &elem : this->_subElements) {
        if(filterType == elem->type()) list.append(elem);
    }

    return list;
}

void AssetsDatabaseElement::rename(const QString &newName) {
    this->_name = newName;

    //redefine paths
    this->_definePath();
}


//////////////
/// DEFINES //
//////////////

void AssetsDatabaseElement::_defineFlags() {
    //flags definition
    switch(this->_type) {
        case InternalContainer:
        case DownloadedContainer:
            this->_flags = Qt::ItemIsEnabled;
            break;
        case Player:
        case Event:
            this->_flags = QFlags<Qt::ItemFlag>(Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsDragEnabled | Qt::ItemIsSelectable);
            break;
        case Object:
        case NPC:
        case FloorBrush:
        case Downloaded:
            this->_flags = QFlags<Qt::ItemFlag>(Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
            break;
        case NPC_Container:
        case FloorBrushContainer:
        case ObjectContainer:
            this->_flags = QFlags<Qt::ItemFlag>(Qt::ItemIsEnabled | Qt::ItemIsDropEnabled);
            break;
        case Folder:
            this->_flags = QFlags<Qt::ItemFlag>(Qt::ItemIsEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
            break;
        default:
            this->_flags = 0;
            break;
    }
}

void AssetsDatabaseElement::_definePath() {

    //assimilated root, let default...
    if(!this->_parentElement) {
        this->_path = "";
        return;
    }

    //generate path
    auto path = QString();
    if(this->isContainer()) {

        if(this->isStaticContainer()) {
            //if is static, dont use name
            path = "/{" + QString::number(this->type()) + "}";
        } else {
            //use name for other container types
            path = "/" + this->displayName();
        }

    }

    //return
    this->_path = this->_parentElement->path() + path;
    this->_defineFullPath();

    //update children paths
    for(auto &elem : this->_subElements) {
        elem->_definePath();
    }
}

void AssetsDatabaseElement::_defineFullPath() {
    this->_fullPath = this->isItem() ? 
                            this->path() + "/" + this->_name : 
                            this->path();
}


void AssetsDatabaseElement::_defineIconPath() {
    this->_iconPath = _iconPathByElementType[this->_type];
}

void AssetsDatabaseElement::_resetSubjacentItemsType(const AssetsDatabaseElement::Type &replacingType, AssetsDatabaseElement* target) {
    //update children
    for(auto elem : target->_subElements) {
        if(elem->isContainer()) {
            AssetsDatabaseElement::_resetSubjacentItemsType(replacingType, elem);
        } else if(elem->isItem()) {
            elem->_setType(replacingType);
        }
    }
}

void AssetsDatabaseElement::_defineParent(AssetsDatabaseElement* parent) {
    
    //if already existing parent, tell him to deref child
    if(this->_parentElement) {
        this->_parentElement->unrefChild(this);

        //reset type if base insert type is different from parent's
        auto replacingType = parent->insertType();
        if(this->insertType() != replacingType) {

            //update self
            if(this->isItem()) {
                this->_setType(replacingType);
            }

            //update children
            AssetsDatabaseElement::_resetSubjacentItemsType(replacingType, this);
        }
    }

    //set new parent
    this->_parentElement = parent;

    //paths-related redefinitions
    this->_definePath();
    this->_defineRootStaticContainer();
    this->_defineIsInternal();
    this->_defineInsertType();
}

void AssetsDatabaseElement::_defineIsContainer() {
    this->_isContainer = this->_containerTypes.contains(this->_type);
}
void AssetsDatabaseElement::_defineIsInternal() {
    this->_isInternal = this->rootStaticContainer() == InternalContainer;
}
void AssetsDatabaseElement::_defineIsRoot() {
    this->_isRoot = this->_type == AssetsDatabaseElement::Type::Root;
}
void AssetsDatabaseElement::_defineIsItem() {
    this->_isItem = this->_itemTypes.contains(this->_type);
}
void AssetsDatabaseElement::_defineIsStaticContainer() {
    this->_isStaticContainer = this->_staticContainerTypes.contains(this->_type);
}
void AssetsDatabaseElement::_defineIsDeletable() {
    this->_isDeletable = this->_deletableItemTypes.contains(this->_type);
}


void AssetsDatabaseElement::_defineRootStaticContainer() {
     
    //if no parent, let default
    if(!this->_parentElement){
        this->_rootStaticContainerType = Unknown;
        return;
    }

    //if self is bound
    if(this->isStaticContainer()) {
        this->_rootStaticContainerType = this->_type;
        return;
    }

    //fetch the information from parent
    this->_rootStaticContainerType = this->_parentElement->isStaticContainer() ? 
            this->_parentElement->type() : 
            this->_parentElement->rootStaticContainer();
}


void AssetsDatabaseElement::_defineInsertType() {
    switch(this->rootStaticContainer()) {
        case NPC_Container:
            this->_insertType = NPC;
            break;
        case FloorBrushContainer:
            this->_insertType = FloorBrush;
            break;
        case ObjectContainer:
            this->_insertType = Object;
            break;
        case DownloadedContainer:
            this->_insertType = Downloaded;
            break;
        default:
            this->_insertType = Unknown;
            break;
    }
}

//////////////////
/// END DEFINES //
//////////////////

//////////////
/// HELPERS //
//////////////


QList<AssetsDatabaseElement::Type> AssetsDatabaseElement::staticContainerTypes() {
    return _staticContainerTypes;
}

QList<AssetsDatabaseElement::Type> AssetsDatabaseElement::internalItemTypes() {
    return _internalItemsTypes;
}

QString AssetsDatabaseElement::typeDescription(AssetsDatabaseElement::Type &type) {
    return _typeDescriptions[type];
}

bool AssetsDatabaseElement::isAcceptableNameChange(QString &newName) {
    
    //strip name from slashes and double quotes
    newName.replace("\"", "");
    newName.replace("/", "");

    //if empty name
    if(newName.isEmpty()) return false;

    //if same name, no changes
    if(this->displayName() == newName) return false;

    return true;
    
}

void AssetsDatabaseElement::sortByPathLengthDesc(QList<AssetsDatabaseElement*> &listToSort) {
    
    //sort algorythm
    struct {
        bool operator()(AssetsDatabaseElement* a, AssetsDatabaseElement* b) const {   
            return a->fullPath().count("/") > b->fullPath().count("/");
        }   
    } pathLength;

    //sort
    std::sort(listToSort.begin(), listToSort.end(), pathLength);

}

QList<QString> AssetsDatabaseElement::pathAsList(QString path) {
    return path.split("/", QString::SplitBehavior::SkipEmptyParts);
}

AssetsDatabaseElement::Type AssetsDatabaseElement::pathChunktoType(QString &chunk) {
    
    auto expected = chunk.startsWith("{") && chunk.endsWith("}");
    if(!expected) {
        qDebug() << "Assets : ignoring path, as its structure is not expected.";
        return Unknown;
    }
    
    //type cast and get element type
    chunk.replace("{", "");
    chunk.replace("}", "");
    auto castOk = false;
    auto staticCType = (AssetsDatabaseElement::Type)chunk.toInt(&castOk);
    if(!castOk) {
        qDebug() << "Assets : ignoring path, as static container type was impossible to deduce";
        return Unknown;
    }

    return staticCType;
}

QSet<AssetsDatabaseElement*> AssetsDatabaseElement::filterTopMostOnly(QList<AssetsDatabaseElement*> elemsToFilter) {

    QSet<AssetsDatabaseElement*> higher;
    while(elemsToFilter.count()) {

        //take first
        if(!higher.count()) {
            higher.insert(elemsToFilter.takeFirst());
            continue;
        }

        //compare
        auto toCompareTo = elemsToFilter.takeFirst();
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

//////////////////
/// END HELPERS //
//////////////////