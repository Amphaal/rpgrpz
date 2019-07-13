#pragma once

#include <QVariantHash>

#include "src/shared/assets/AssetsDatabaseElement.h"
#include "src/shared/database/AssetsDatabase.h"

class AssetMetadata : public QVariantHash {
    public:
        AssetMetadata();
        AssetMetadata(const QVariantHash &hash);
        AssetMetadata(AssetsDatabaseElement *elem);
        AssetMetadata(const QString &pathToAssetFile);

        QString pathToAssetFile();
        QString assetName();
        RPZAssetHash assetId();
        AtomType atomType();
        int brushWidth();

        void setBrushWidth(int width);
    
    private:
        void _setAtomType(const AtomType &type);

        void _setFilePath(const QString &pathToAssetFile);
};