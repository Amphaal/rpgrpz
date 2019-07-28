#pragma once

#include <QVariantHash>

#include "src/shared/assets/AssetsDatabaseElement.h"
#include "src/shared/database/AssetsDatabase.h"

class RPZAssetMetadata : public QVariantHash {
    public:
        RPZAssetMetadata();
        RPZAssetMetadata(const QVariantHash &hash);
        RPZAssetMetadata(AssetsDatabaseElement *elem);
        RPZAssetMetadata(const RPZAssetHash &assetId, const QString &pathToAssetFile);

        QString pathToAssetFile() const;
        QString assetName() const;
        RPZAssetHash assetId() const;
        AtomType atomType() const;
        int brushWidth() const;

        void setBrushWidth(int width);
    
    private:
        void _setAtomType(const AtomType &type);
        void _setFilePath(const QString &pathToAssetFile);
};