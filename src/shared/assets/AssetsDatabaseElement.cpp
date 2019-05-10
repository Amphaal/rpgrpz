#include "AssetsDatabaseElement.h"

AssetsDatabaseElement::AssetsDatabaseElement(
    const QString &name, 
    const AssetsDatabaseElement::Type &type
) : _name(name), _type(type) { };

AssetsDatabaseElement::AssetsDatabaseElement() : AssetsDatabaseElement("", AssetsDatabaseElement::Type::Root) { };

AssetsDatabaseElement::~AssetsDatabaseElement(){
    qDeleteAll(this->_subElements);
}

QString AssetsDatabaseElement::getIconPathForType(const AssetsDatabaseElement::Type &type) {
    return _iconPathByElementType[type];
}

QString AssetsDatabaseElement::displayName() {
    return this->_name;
}

AssetsDatabaseElement::Type AssetsDatabaseElement::type() {
    return this->_type;
}

QString AssetsDatabaseElement::iconPath() {
    return getIconPathForType(this->type());
}

AssetsDatabaseElement* AssetsDatabaseElement::parent() {
    return this->_parentElement;
}

int AssetsDatabaseElement::row() const {
    if (this->_parentElement) {
        return this->_parentElement->_subElements.indexOf(const_cast<AssetsDatabaseElement*>(this));
    }

    return 0;
}

QString AssetsDatabaseElement::path() {

    //assimilated root...
    if(!this->_parentElement) return "";

    //generate path
    auto path = QString();
    if(this->isContainer()) {

        if(this->_staticContainerTypes.contains(this->type())) {
            //if is static, dont use name
            path = "/{" + QString::number(this->type()) + "}";
        } else {
            //use name for other container types
            path = "/" + this->displayName();
        }

    }

    //return
    return this->_parentElement->path() + path;
}

AssetsDatabaseElement* AssetsDatabaseElement::child(int row) {
    return this->_subElements.value(row);
}

int AssetsDatabaseElement::childCount() const {
    return this->_subElements.count();
}

bool AssetsDatabaseElement::isContainer() {
    return _containerTypes.contains(this->_type);
}

bool AssetsDatabaseElement::isInternal() {
    return this->getBoundStaticContainer() == InternalContainer;
}

bool AssetsDatabaseElement::isRoot() {
    return this->_type == AssetsDatabaseElement::Type::Root;
}

void AssetsDatabaseElement::appendChild(AssetsDatabaseElement* child) {
    child->defineParent(this);
    this->_subElements.append(child);
};

AssetsDatabaseElement::Type AssetsDatabaseElement::defaultTypeOnContainerForInsert() {
    switch(this->getBoundStaticContainer()) {
        case NPC_Container:
            return NPC;
            break;
        case FloorBrushContainer:
            return FloorBrush;
            break;
        case ObjectContainer:
            return Object;
            break;
        default:
            return Unknown;
    }
}

void AssetsDatabaseElement::defineParent(AssetsDatabaseElement* parent) {
    this->_parentElement = parent;
}

AssetsDatabaseElement::Type AssetsDatabaseElement::getBoundStaticContainer() {
    
    //if self is bound
    if(_staticContainerTypes.contains(this->_type)) return this->_type;
    
    //if no parent, return unkown
    if(!this->_parentElement) return Unknown;
    
    //fetch the information from parent
    auto parentType = this->_parentElement->type();
    return _staticContainerTypes.contains(parentType) ? parentType : this->_parentElement->getBoundStaticContainer();
}

Qt::ItemFlags AssetsDatabaseElement::flags() {
    switch(this->_type) {
        case InternalContainer:
            return Qt::ItemIsEnabled;
            break;
        case Player:
        case Event:
        case Object:
        case NPC:
        case FloorBrush:
            return QFlags<Qt::ItemFlag>(Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsDragEnabled | Qt::ItemIsSelectable);
            break;
        case NPC_Container:
        case FloorBrushContainer:
        case ObjectContainer:
            return QFlags<Qt::ItemFlag>(Qt::ItemIsEnabled | Qt::ItemIsDropEnabled);
            break;
        case Folder:
            return QFlags<Qt::ItemFlag>(Qt::ItemIsEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled);
        default:
            return 0;
            break;
    }
}
