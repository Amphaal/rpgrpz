#include "MapLayoutCategory.h"
#include "MapLayoutAtom.h"

MapLayoutAtom::MapLayoutAtom(MapLayoutCategory* parent, const RPZAtom &atom) {
    
    this->_id = atom.id();
    this->_type = atom.type();
    this->_assetHash = atom.assetHash();
    this->_nameChangeParam = RPZAtom::descriptorsByAtomType.value(this->_type);
    this->_name = atom.toString();
    this->_iconPath = atom.descriptiveIconPath();

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

const QHash<int, QSet<int>> MapLayoutAtom::updateFrom(const RPZAtom::Updates &updates) {
    
    QHash<int, QSet<int>> colsByDataRoles;

    for(auto i = updates.begin(); i != updates.end(); i++) {
        
        auto param = i.key();
        auto variant = i.value();
        
        switch (param) {

            case RPZAtom::Parameter::Hidden:
                this->_isHidden = variant.toBool();
                colsByDataRoles[1].insert(Qt::DisplayRole);
                break;
            
            case RPZAtom::Parameter::Locked:
                this->_isLocked = variant.toBool();
                colsByDataRoles[1].insert(Qt::DisplayRole);
                break;
            
            case RPZAtom::Parameter::NPCAttitude:
                this->_iconPath = RPZAtom::descriptiveIconPath(this->_type, (RPZAtom::NPCType)variant.toInt());
                colsByDataRoles[0].insert(Qt::DecorationRole);
                break;

            default:
                break;

        }

        //rename ?        
        if(param == this->_nameChangeParam && param != RPZAtom::Parameter::Unknown) {
            
            this->_name = RPZAtom::toString(
                this->_type,
                variant.toString()
            );

            colsByDataRoles[0].insert(Qt::DisplayRole);

        } 

    }

    return colsByDataRoles;

}

bool MapLayoutAtom::notifyAssetNameChange(const QString newAssetName) {

    if(this->_nameChangeParam != RPZAtom::Parameter::AssetName) return false;

    this->_name = RPZAtom::toString(
        this->_type,
        newAssetName
    );

    return true;

}

bool MapLayoutAtom::isHidden() const {
    return this->_isHidden;
}

bool MapLayoutAtom::isLocked() const {
    return this->_isLocked;
}

const RPZAsset::Hash MapLayoutAtom::assetHash() const {
    return this->_assetHash;
}

const RPZAtom::Id MapLayoutAtom::atomId() const {
    return this->_id;
}

const QString MapLayoutAtom::name() const {
    return this->_name;
}

const QPixmap MapLayoutAtom::icon() const { 
    return QPixmap(this->_iconPath); 
}


