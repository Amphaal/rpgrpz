#include "AssetMetadata.h"

AssetMetadata::AssetMetadata() {}
AssetMetadata::AssetMetadata(const QVariantHash &hash) :  QVariantHash(hash) {}
AssetMetadata::AssetMetadata(AssetsDatabaseElement *elem) {
    this->_setAtomType(elem->atomType());

    if(elem->isItem()) {
        this->insert("aId", elem->id());
        this->insert("nm", elem->displayName());
        this->insert("pth", AssetsDatabase::get()->getFilePathToAsset(elem));
    } 

}
AssetMetadata::AssetMetadata(const RPZAssetHash &assetId, const QString &pathToAssetFile) {
    this->insert("aId", assetId);
    this->_setFilePath(pathToAssetFile);
}

QString AssetMetadata::pathToAssetFile() {
    return this->value("pth").toString();
}

QString AssetMetadata::assetName() {
    return this->value("nm").toString();
}

RPZAssetHash AssetMetadata::assetId() {
    return this->value("aId").toString();
}

AtomType AssetMetadata::atomType() {
    return (AtomType)this->value("t").toInt();
}

int AssetMetadata::brushWidth() {
    return this->value("bw").toInt();
}

void AssetMetadata::setBrushWidth(int width) {
    this->insert("bw", width);
}

void AssetMetadata::_setAtomType(const AtomType &type) {
    this->insert("t", (int)type);
}

void AssetMetadata::_setFilePath(const QString &pathToAssetFile) {
    this->insert("pth", pathToAssetFile);
}