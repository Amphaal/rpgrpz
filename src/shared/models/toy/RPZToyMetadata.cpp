#include "RPZToyMetadata.h"

RPZToyMetadata::RPZToyMetadata() {}
RPZToyMetadata::RPZToyMetadata(const QVariantHash &hash) : QVariantHash(hash) {}

RPZToyMetadata::RPZToyMetadata(const AtomType &type) {
    this->setAtomType(type);
}

RPZToyMetadata::RPZToyMetadata(
        AssetsTreeViewItem* parentContainer,
        const AtomType &type,
        const RPZAssetHash &assetId, 
        const QString &assetName, 
        const QString &pathToAsset,
        const QSize &dimensions,
        const QPointF &center
    ) : _parentContainer(parentContainer) {
    
    this->insert(QStringLiteral(u"aId"), assetId);
    this->insert(QStringLiteral(u"nm"), assetName);
    this->insert(QStringLiteral(u"shp"), dimensions);
    this->insert(QStringLiteral(u"ctr"), center);
    this->insert(QStringLiteral(u"pth"), pathToAsset);

    this->setAtomType(type);

}

QPointF RPZToyMetadata::center() const {
    return this->value(QStringLiteral(u"ctr")).toPointF();
}

QSize RPZToyMetadata::shapeSize() const {
    return this->value(QStringLiteral(u"shp")).toSize();
}

AssetsTreeViewItem* RPZToyMetadata::associatedParent() const {
    return this->_parentContainer;
}

QString RPZToyMetadata::pathToAssetFile() const {
    return this->value(QStringLiteral(u"pth")).toString();
}

QString RPZToyMetadata::assetName() const {
    return this->value(QStringLiteral(u"nm")).toString();
}

RPZAssetHash RPZToyMetadata::assetId() const {
    return this->value(QStringLiteral(u"aId")).toString();
}

AtomType RPZToyMetadata::atomType() const {
    return (AtomType)this->value(QStringLiteral(u"t")).toInt();
}

void RPZToyMetadata::setAtomType(const AtomType &type) {
    this->insert(QStringLiteral(u"t"), (int)type);
}