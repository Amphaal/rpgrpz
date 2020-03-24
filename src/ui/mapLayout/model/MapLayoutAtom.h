#pragma once

#include "MapLayoutItem.hpp"

#include "src/shared/models/RPZAtom.h"

class MapLayoutCategory;
class MapLayoutAtom : public MapLayoutItem {
    public:
        MapLayoutAtom(MapLayoutCategory* parent, const RPZAtom &atom);
        ~MapLayoutAtom();

        static MapLayoutAtom* fromIndex(const QModelIndex &index);

        void setParent(MapLayoutCategory* parent);
        MapLayoutCategory* parent() const;

        const QHash<int, QSet<int>> updateFrom(const RPZAtom::Updates &updates);

        bool isHidden() const;
        bool isLocked() const;
        const RPZAsset::Hash assetHash() const;
        RPZAtom::Id atomId() const;
        const QString name() const override;
        const QPixmap icon() const override;

        bool notifyAssetNameChange(const QString newAssetName);

    private:
        MapLayoutCategory* _parent = nullptr;
        QString _name;
        RPZAtom::Type _type = (RPZAtom::Type)0;
        bool _isHidden = false;
        bool _isLocked = false;
        RPZAtom::Id _id = 0;
        RPZAsset::Hash _assetHash;
        QString _iconPath;

        RPZAtom::Parameter _nameChangeParam;

};