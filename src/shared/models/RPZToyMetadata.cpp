#include "RPZToyMetadata.h"

RPZToyMetadata::RPZToyMetadata() {}

RPZToyMetadata::RPZToyMetadata(const AtomType &type) {
    this->_setAtomType(type);
}

RPZToyMetadata::RPZToyMetadata(
        AssetsDatabaseElement* parentContainer,
        const AtomType &type,
        const RPZAssetHash &assetId, 
        const QString &assetName, 
        const QString &pathToAsset,
        const QSize &dimensions,
        const QPointF &center
    ) : _parentContainer(parentContainer) {
    
    this->insert("aId", assetId);
    this->insert("nm", assetName);
    this->insert("shp", dimensions);
    this->insert("ctr", center);

    this->_setFilePath(pathToAsset);
    this->_setAtomType(type);

}

QPointF RPZToyMetadata::center() const {
    return this->value("ctr").toPointF();
}

QSize RPZToyMetadata::shapeSize() const {
    return this->value("shp").toSize();
}

AssetsDatabaseElement* RPZToyMetadata::associatedParent() const {
    return this->_parentContainer;
}

QString RPZToyMetadata::pathToAssetFile() const {
    return this->value("pth").toString();
}

QString RPZToyMetadata::assetName() const {
    return this->value("nm").toString();
}

RPZAssetHash RPZToyMetadata::assetId() const {
    return this->value("aId").toString();
}

AtomType RPZToyMetadata::atomType() const {
    return (AtomType)this->value("t").toInt();
}

void RPZToyMetadata::_setAtomType(const AtomType &type) {
    this->insert("t", (int)type);
}

void RPZToyMetadata::_setFilePath(const QString &pathToAssetFile) {
    this->insert("pth", pathToAssetFile);
}