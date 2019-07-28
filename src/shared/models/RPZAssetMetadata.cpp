#include "RPZAssetMetadata.h"

RPZAssetMetadata::RPZAssetMetadata() {}
RPZAssetMetadata::RPZAssetMetadata(const QVariantHash &hash) :  QVariantHash(hash) {}
RPZAssetMetadata::RPZAssetMetadata(AssetsDatabaseElement *elem) {
    this->_setAtomType(elem->atomType());

    if(elem->isItem()) {
        this->insert("aId", elem->id());
        this->insert("nm", elem->displayName());
        this->insert("pth", AssetsDatabase::get()->getFilePathToAsset(elem));
    } 

}
RPZAssetMetadata::RPZAssetMetadata(const RPZAssetHash &assetId, const QString &pathToAssetFile) {
    this->insert("aId", assetId);
    this->_setFilePath(pathToAssetFile);
}

QString RPZAssetMetadata::pathToAssetFile() const {
    return this->value("pth").toString();
}

QString RPZAssetMetadata::assetName() const {
    return this->value("nm").toString();
}

RPZAssetHash RPZAssetMetadata::assetId() const {
    return this->value("aId").toString();
}

AtomType RPZAssetMetadata::atomType() const {
    return (AtomType)this->value("t").toInt();
}

int RPZAssetMetadata::brushWidth() const {
    return this->value("bw").toInt();
}

void RPZAssetMetadata::setBrushWidth(int width) {
    this->insert("bw", width);
}

void RPZAssetMetadata::_setAtomType(const AtomType &type) {
    this->insert("t", (int)type);
}

void RPZAssetMetadata::_setFilePath(const QString &pathToAssetFile) {
    this->insert("pth", pathToAssetFile);
}