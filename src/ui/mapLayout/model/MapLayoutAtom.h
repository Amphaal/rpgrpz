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

        const QSet<int> updateFrom(const RPZAtom::Updates &updates);

        bool isHidden() const;

        bool isLocked() const;
        
        const RPZAsset::Hash assetHash() const;
        
        const RPZAtom::Id atomId() const;

        const QString name() const override;

        void setName(const QString assetName = QString());

        const QPixmap icon() const override;

    private:
        MapLayoutCategory* _parent = nullptr;
        QString _name;
        RPZAtom::Type _type = (RPZAtom::Type)0;
        bool _isHidden = false;
        bool _isLocked = false;
        RPZAtom::Id _id = 0;
        RPZAsset::Hash _assetHash;

};