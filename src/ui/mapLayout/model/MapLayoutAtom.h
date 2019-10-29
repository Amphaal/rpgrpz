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

        const QSet<int> updateFrom(const AtomUpdates &updates);

        bool isHidden() const;

        bool isLocked() const;
        
        const RPZAssetHash assetHash() const;
        
        const RPZAtomId atomId() const;

        const QString name() const override;

        void setName(const QString assetName = QString());

        const QPixmap icon() const override;

    private:
        MapLayoutCategory* _parent = nullptr;
        QString _name;
        RPZAtomType _type = (RPZAtomType)0;
        bool _isHidden = false;
        bool _isLocked = false;
        RPZAtomId _id = 0;
        RPZAssetHash _assetHash;

};