#include "AssetsTreeViewItem.h"

AssetsTreeViewItem::AssetsTreeViewItem(const RPZAsset* asset, AssetsTreeViewItem* parent) : AssetsTreeViewItem(
        asset->name(), 
        parent, 
        parent->insertType()
    ) {
 
    this->_hash = asset->hash();

}

AssetsTreeViewItem::AssetsTreeViewItem() : AssetsTreeViewItem(QString(), nullptr, AssetsTreeViewItem::Type::Root) {};

AssetsTreeViewItem::~AssetsTreeViewItem(){
    
    if(this->_parentElement) {
        this->_parentElement->unrefChild(this);
    }

    qDeleteAll(this->_subElements);
}

AssetsTreeViewItem::AssetsTreeViewItem(
    const QString &name, 
    AssetsTreeViewItem* parent,
    const AssetsTreeViewItem::Type &type
) { 
    //define type
    this->_setType(type);

    //define name (fullpath redefinition included)
    this->rename(name);

   //if a parent is defined, add self to its inner list
    if(parent) {
        parent->appendChild(this);
    }

};

void AssetsTreeViewItem::_setType(const AssetsTreeViewItem::Type &type) {
    this->_type = type; 

    // types-related definitions
    this->_defineAtomType();
    this->_defineIconPath();
    this->_defineFlags();
    this->_defineIsContainer();
    this->_defineIsRoot();
    this->_defineIsIdentifiable();
    this->_defineIsStaticContainer();
    this->_defineIsDeletable();

    //redefine RPZAsset
    if(!this->_asset.isEmpty()) {
        this->_asset.setAtomType(this->atomType());
    }
    
}

///////////////////
// RO Properties //
///////////////////

const RPZAsset* AssetsTreeViewItem::asset() const {
    return AssetsDatabase::get()->asset();
}

const Qt::ItemFlags AssetsTreeViewItem::flags() const {
    return this->_flags;
}

const QString AssetsTreeViewItem::displayName() const {
    return this->_name;
}

const AssetsTreeViewItem::Type AssetsTreeViewItem::type() const {
    return this->_type;
}

const RPZAtomType AssetsTreeViewItem::atomType() const {
    return this->_atomType;
}

AssetsTreeViewItem* AssetsTreeViewItem::parent() {
    return this->_parentElement;
}

const QString AssetsTreeViewItem::path() const {
    return this->_path;
}

const QString AssetsTreeViewItem::iconPath() const {
    return this->_iconPath;
}

bool AssetsTreeViewItem::isContainer() const {
    return this->_isContainer;
}

bool AssetsTreeViewItem::isInternal() const {
    return this->_isInternal;
}

bool AssetsTreeViewItem::isRoot() const {
    return this->_isRoot;
}

bool AssetsTreeViewItem::isIdentifiable() const {
    return this->_isIdentifiable;
}

bool AssetsTreeViewItem::isDeletable() const {
    return this->_isDeletable;
}

const AssetsTreeViewItem::Type AssetsTreeViewItem::insertType() const {
    return this->_insertType;
}

const AssetsTreeViewItem::Type AssetsTreeViewItem::rootStaticContainer() const {
    return this->_rootStaticContainerType;
}

///////////////////////
// END RO Properties //
///////////////////////

bool AssetsTreeViewItem::contains(AssetsTreeViewItem* toCheck, AssetsTreeViewItem* toBeChecked) {
    if(!toBeChecked) toBeChecked = this;
    
    //check if self
    if(toBeChecked == toCheck) return true;

    //check children
    auto currentDoesContainInChildren = toBeChecked->_subElements.contains(toCheck);
    if(currentDoesContainInChildren) return true;

    return false;
}

bool AssetsTreeViewItem::containsAny(const QList<AssetsTreeViewItem*> toCheck) {
    
    for(auto i : toCheck) {
        auto doesContain = this->contains(i);
        if(doesContain) return true;
    }

    return false;

}

AssetsTreeViewItem* AssetsTreeViewItem::child(int row) {
    return this->_subElements.value(row);
}

int AssetsTreeViewItem::childCount() const {
    return this->_subElements.count();
}

int AssetsTreeViewItem::itemChildrenCount() const {
    return this->_itemChildrenCount;
}

int AssetsTreeViewItem::row() const {
    if (this->_parentElement) {
        return this->_parentElement->_subElements.indexOf(const_cast<AssetsTreeViewItem*>(this));
    }

    return 0;
}

void AssetsTreeViewItem::appendChild(AssetsTreeViewItem* child) {
    
    child->_defineParent(this);

    //add to list
    if(child->type() == AssetsTreeViewItem::Type::Folder) {
        this->_subElements.prepend(child);
    } else {
        this->_subElements.append(child);
    }
    
    //increment count
    if(!child->isContainer()) this->_itemChildrenCount++;
};


void AssetsTreeViewItem::unrefChild(AssetsTreeViewItem* child) {

    //find child in subelements
    auto foundIndex = this->_subElements.indexOf(child);
    
    //if found
    if (foundIndex > -1) {

        //unref
        this->_subElements.removeAt(foundIndex);

        //unincrement
        if(!child->isContainer()) _itemChildrenCount--;
    }
}

QList<AssetsTreeViewItem*> AssetsTreeViewItem::childrenContainers() {
    QList<AssetsTreeViewItem*> list;
    
    for(auto &elem : this->_subElements) {
        if(elem->isContainer()) list.append(elem);
    }

    return list;
}

QList<AssetsTreeViewItem*> AssetsTreeViewItem::childrenItems() {
    QList<AssetsTreeViewItem*> list;
    auto filterType = this->insertType();
    
    for(auto &elem : this->_subElements) {
        if(filterType == elem->type()) list.append(elem);
    }

    return list;
}

void AssetsTreeViewItem::rename(const QString &newName) {
    this->_name = newName;

    //redefine paths
    this->_definePath();
}


//////////////
/// DEFINES //
//////////////

void AssetsTreeViewItem::_defineFlags() {

    //flags definition
    switch(this->_type) {
        
        case AssetsTreeViewItem::Type::InternalContainer:
        case AssetsTreeViewItem::Type::DownloadedContainer:
            this->_flags = Qt::ItemIsEnabled;
            break;
        
        //disabled for now
        case AssetsTreeViewItem::Type::Text:
        case AssetsTreeViewItem::Type::Event:
        case AssetsTreeViewItem::Type::NPC_Container:
        case AssetsTreeViewItem::Type::BackgroundContainer:
        case AssetsTreeViewItem::Type::Background:
        case AssetsTreeViewItem::Type::NPC:
            this->_flags = QFlags<Qt::ItemFlag>(Qt::ItemNeverHasChildren);
            break;
        
        case AssetsTreeViewItem::Type::FreeDraw:
            this->_flags = QFlags<Qt::ItemFlag>(Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsDragEnabled | Qt::ItemIsSelectable);
            break;
        
        case AssetsTreeViewItem::Type::Object:
        case AssetsTreeViewItem::Type::FloorBrush:
        case AssetsTreeViewItem::Type::Downloaded:
        // case Background:
        // case NPC:
            this->_flags = QFlags<Qt::ItemFlag>(Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
            break;
        
        case AssetsTreeViewItem::Type::FloorBrushContainer:
        case AssetsTreeViewItem::Type::ObjectContainer:
        // case NPC_Container:
        // case BackgroundContainer:
            this->_flags = QFlags<Qt::ItemFlag>(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled);
            break;
        
        case AssetsTreeViewItem::Type::Folder:
            this->_flags = QFlags<Qt::ItemFlag>(Qt::ItemIsEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
            break;
        
        default:
            this->_flags = 0;
            break;
    }
}

void AssetsTreeViewItem::_defineAtomType() {
    this->_atomType = toAtomType(this->_type);
}

void AssetsTreeViewItem::_definePath() {

    //assimilated root, let default...
    if(!this->_parentElement) {
        this->_path = "";
        return;
    }

    //generate path
    auto path = QString();
    if(this->isContainer()) {

        if(this->_isStaticContainer) {
            //if is static, dont use name
            path = "/{" + QString::number((int)this->type()) + "}";
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

void AssetsTreeViewItem::_defineFullPath() {
    this->_fullPath = this->isIdentifiable() ? 
                            this->path() + "/" + this->_name : 
                            this->path();
}


void AssetsTreeViewItem::_defineIconPath() {
    this->_iconPath = _iconPathByElementType.value(this->_type);
}

void AssetsTreeViewItem::_resetSubjacentItemsType(const AssetsTreeViewItem::Type &replacingType, AssetsTreeViewItem* target) {
    //update children
    for(auto elem : target->_subElements) {
        if(elem->isContainer()) {
            AssetsTreeViewItem::_resetSubjacentItemsType(replacingType, elem);
        } else if(elem->isIdentifiable()) {
            elem->_setType(replacingType);
        }
    }
}

void AssetsTreeViewItem::_defineParent(AssetsTreeViewItem* parent) {
    
    //if already existing parent, tell him to deref child
    if(this->_parentElement) {
        this->_parentElement->unrefChild(this);

        //reset type if base insert type is different from parent's
        auto replacingType = parent->insertType();
        if(this->insertType() != replacingType) {

            //update self
            if(this->isIdentifiable()) {
                this->_setType(replacingType);
            }

            //update children
            AssetsTreeViewItem::_resetSubjacentItemsType(replacingType, this);
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

void AssetsTreeViewItem::_defineIsContainer() {
    this->_isContainer = this->_staticContainerTypes.contains(this->_type) || this->_type == AssetsTreeViewItem::Type::Folder;
}
void AssetsTreeViewItem::_defineIsInternal() {
    this->_isInternal = this->rootStaticContainer() == AssetsTreeViewItem::Type::InternalContainer;
}
void AssetsTreeViewItem::_defineIsRoot() {
    this->_isRoot = this->_type == AssetsTreeViewItem::Type::Root;
}
void AssetsTreeViewItem::_defineIsIdentifiable() {
    this->_isIdentifiable = this->_itemTypes.contains(this->_type);
}
void AssetsTreeViewItem::_defineIsStaticContainer() {
    this->_isStaticContainer = this->_staticContainerTypes.contains(this->_type);
}
void AssetsTreeViewItem::_defineIsDeletable() {
    this->_isDeletable = this->_itemTypes.contains(this->_type) || this->_type == AssetsTreeViewItem::Type::Folder;
}


void AssetsTreeViewItem::_defineRootStaticContainer() {
     
    //if no parent, let default
    if(!this->_parentElement){
        this->_rootStaticContainerType = AssetsTreeViewItem::Type::T_Unknown;
        return;
    }

    //if self is bound
    if(this->_isStaticContainer) {
        this->_rootStaticContainerType = this->_type;
        return;
    }

    //fetch the information from parent
    this->_rootStaticContainerType = this->_parentElement->_isStaticContainer ? 
            this->_parentElement->type() : 
            this->_parentElement->rootStaticContainer();
}


void AssetsTreeViewItem::_defineInsertType() {
    switch(this->rootStaticContainer()) {
        case AssetsTreeViewItem::Type::NPC_Container:
            this->_insertType = AssetsTreeViewItem::Type::NPC;
            break;
        case AssetsTreeViewItem::Type::FloorBrushContainer:
            this->_insertType = AssetsTreeViewItem::Type::FloorBrush;
            break;
        case AssetsTreeViewItem::Type::ObjectContainer:
            this->_insertType = AssetsTreeViewItem::Type::Object;
            break;
        case AssetsTreeViewItem::Type::DownloadedContainer:
            this->_insertType = AssetsTreeViewItem::Type::Downloaded;
            break;
        case AssetsTreeViewItem::Type::BackgroundContainer:
            this->_insertType = AssetsTreeViewItem::Type::Background;
            break;
        default:
            this->_insertType = AssetsTreeViewItem::Type::T_Unknown;
            break;
    }
}

//////////////////
/// END DEFINES //
//////////////////

//////////////
/// HELPERS //
//////////////


QList<AssetsTreeViewItem::Type> AssetsTreeViewItem::staticContainerTypes() {
    return _staticContainerTypes;
}

QList<AssetsTreeViewItem::Type> AssetsTreeViewItem::movableStaticContainerTypes() {
    return _movableStaticContainerTypes;
}

QList<AssetsTreeViewItem::Type> AssetsTreeViewItem::internalItemTypes() {
    return _internalItemsTypes;
}

QString AssetsTreeViewItem::typeDescription(AssetsTreeViewItem::Type &type) {
    return tr(qUtf8Printable(_typeDescriptions.value(type)));
}

bool AssetsTreeViewItem::isAcceptableNameChange(QString &newName) {
    
    //strip name from slashes and double quotes
    newName.replace("\"", "");
    newName.replace("/", "");

    //if empty name
    if(newName.isEmpty()) return false;

    //if same name, no changes
    if(this->displayName() == newName) return false;

    return true;
    
}

void AssetsTreeViewItem::sortByPathLengthDesc(QList<AssetsTreeViewItem*> &listToSort) {
    
    //sort algorythm
    struct {
        bool operator()(AssetsTreeViewItem* a, AssetsTreeViewItem* b) const {   
            return a->_fullPath.count("/") > b->_fullPath.count("/");
        }   
    } pathLength;

    //sort
    std::sort(listToSort.begin(), listToSort.end(), pathLength);

}

QList<QString> AssetsTreeViewItem::pathAsList(const QString &path) {
    return path.split("/", QString::SplitBehavior::SkipEmptyParts);
}

AssetsTreeViewItem::Type AssetsTreeViewItem::pathChunktoType(const QString &chunk) {
    
    auto expected = chunk.startsWith("{") && chunk.endsWith("}");
    if(!expected) {
        qDebug() << "Assets : ignoring path, as its structure is not expected.";
        return AssetsTreeViewItem::Type::T_Unknown;
    }
    
    //type cast and get element type
    auto cp_chunk = chunk;
    cp_chunk.replace("{", "");
    cp_chunk.replace("}", "");
    auto castOk = false;
    auto staticCType = (AssetsTreeViewItem::Type)cp_chunk.toInt(&castOk);
    if(!castOk) {
        qDebug() << "Assets : ignoring path, as static container type was impossible to deduce";
        return AssetsTreeViewItem::Type::T_Unknown;
    }

    return staticCType;
}

QSet<AssetsTreeViewItem*> AssetsTreeViewItem::filterTopMostOnly(QList<AssetsTreeViewItem*> elemsToFilter) {

    QSet<AssetsTreeViewItem*> higher;
    while(elemsToFilter.count()) {

        //take first
        if(!higher.count()) {
            higher.insert(elemsToFilter.takeFirst());
            continue;
        }

        //compare
        auto toCompareTo = elemsToFilter.takeFirst();
            auto compare_path = toCompareTo->_fullPath;
            auto compare_length = compare_path.length();
        
        auto isForeigner = true;

        //iterate
        QSet<AssetsTreeViewItem*> obsoletePointers;
        for(auto &st : higher) {

            auto st_path = st->_fullPath;
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

AssetsTreeViewItem* AssetsTreeViewItem::fromIndex(const QModelIndex &index) {
    auto ip = index.internalPointer();
    return static_cast<AssetsTreeViewItem*>(ip);
}