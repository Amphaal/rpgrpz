#pragma once

#include <QVariantHash>

#include "src/shared/models/RPZAtom.h"

class AssetsTreeViewItem;

class RPZToyMetadata : public QVariantHash {
    public:
        RPZToyMetadata();
        RPZToyMetadata(const QVariantHash &hash);
        RPZToyMetadata(
            AssetsTreeViewItem* parentContainer,
            const AtomType &type, 
            const RPZAssetHash &assetId, 
            const QString &assetName, 
            const QString &pathToAsset,
            const QSize &dimensions,
            const QPointF &center
        );
        RPZToyMetadata(const AtomType &type);

        QString pathToAssetFile() const;
        QString assetName() const;
        RPZAssetHash assetId() const;
        AtomType atomType() const;
        QPointF center() const;
        QSize shapeSize() const;

        AssetsTreeViewItem* associatedParent() const;

        void setAtomType(const AtomType &type);

    private:
        AssetsTreeViewItem* _parentContainer = nullptr;
};