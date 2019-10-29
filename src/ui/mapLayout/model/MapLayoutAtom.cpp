#include "MapLayoutCategory.h"
#include "MapLayoutAtom.h"

MapLayoutAtom::MapLayoutAtom(MapLayoutCategory* parent, const RPZAtom &atom) {
    
    this->_id = atom.id();
    this->_type = atom.type();
    this->_assetHash = atom.assetHash();
    
    //if no hash associated, force descr
    if(this->_assetHash.isEmpty()) {
        this->setName();
    }

    this->updateFrom(atom.editedMetadataWithValues());

    this->setParent(parent);

}

MapLayoutAtom::~MapLayoutAtom() {
    this->setParent(nullptr);
}

MapLayoutAtom* MapLayoutAtom::fromIndex(const QModelIndex &index) {
    auto ip = index.internalPointer();
    return static_cast<MapLayoutAtom*>(ip);
};

void MapLayoutAtom::setParent(MapLayoutCategory* parent) {
    if(this->_parent) this->_parent->removeAsChild(this);
    this->_parent = parent;
    if(parent) parent->addAsChild(this);
}

MapLayoutCategory* MapLayoutAtom::parent() const {
    return this->_parent;
}

const QSet<int> MapLayoutAtom::updateFrom(const AtomUpdates &updates) {
    
    QSet<int> columnsToUpdate;

    for(auto i = updates.begin(); i != updates.end(); i++) {
        
        auto param = i.key();
        auto variant = i.value();
        
        switch (param) {

            case AtomParameter::Hidden:
                this->_isHidden = variant.toBool();
                columnsToUpdate += 1;
                break;
            
            case AtomParameter::Locked:
                this->_isLocked = variant.toBool();
                columnsToUpdate += 1;
                break;
            
            case AtomParameter::AssetName:
                this->setName(variant.toString());
                columnsToUpdate += 0;
                break;

            default:
                break;
        }
    }

    return columnsToUpdate;

}

bool MapLayoutAtom::isHidden() const {
    return this->_isHidden;
}

bool MapLayoutAtom::isLocked() const {
    return this->_isLocked;
}

const RPZAssetHash MapLayoutAtom::assetHash() const {
    return this->_assetHash;
}

const RPZAtomId MapLayoutAtom::atomId() const {
    return this->_id;
}

const QString MapLayoutAtom::name() const {
    return this->_name;
}

void MapLayoutAtom::setName(const QString assetName) {
    this->_name = RPZAtom::toString(
        this->_type,
        assetName
    );
}

const QPixmap MapLayoutAtom::icon() const { 
    return QPixmap(RPZAtom::iconPathByAtomType.value(this->_type)); 
}


