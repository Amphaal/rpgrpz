#pragma once

#include <QVariantHash>

#include "src/shared/assets/AssetsDatabaseElement.h"
#include "src/shared/database/AssetsDatabase.h"

class AssetMetadata : public QVariantHash {
    public:
        AssetMetadata() {}
        AssetMetadata(const QVariantHash &hash) :  QVariantHash(hash) {}
        AssetMetadata(AssetsDatabaseElement *elem) {
            this->_setAtomType(elem->atomType());
            (*this)["aId"] = elem->id();
            (*this)["nm"] = elem->displayName();
            (*this)["pth"] = AssetsDatabase::get()->getFilePathToAsset(elem);
        }

        AssetMetadata(const QString &pathToAssetFile) {
            this->_setFilePath(pathToAssetFile);
        }

        QString pathToAssetFile() {
            return this->value("pth").toString();
        }

        QString assetName() {
            return this->value("nm").toString();
        }

        QString assetId() {
            return this->value("aId").toString();
        }

        AtomType atomType() {
            return (AtomType)this->value("t").toInt();
        }
    
    private:
        void _setAtomType(const AtomType &type) {
            (*this)["t"] = (int)type;
        }

        void _setFilePath(const QString &pathToAssetFile) {
            (*this)["pth"] = pathToAssetFile;
        }
};